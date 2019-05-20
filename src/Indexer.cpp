/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Indexer.h"
#include "ActualRunner.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <zlib.h>
#include <experimental/filesystem>

using std::experimental::filesystem::path;

const unsigned int Indexer::INDEXER_VERSION = 1;

u_int32_t Indexer::calculateIndexBlockInterval(u_int64_t fileSize) {
    const u_int64_t GB = 1024 * 1024 * 1024;
    u_int64_t testSize = fileSize / GB;

    if (testSize <= 1)
        return 16;
    if (testSize <= 2)
        return 32;
    if (testSize <= 4)
        return 64;
    if (testSize <= 8)
        return 128;
    if (testSize <= 16)
        return 256;
    if (testSize <= 32)
        return 512;
    if (testSize <= 64)
        return 1024;
    if (testSize <= 128)
        return 2048;
    if (testSize <= 256)
        return 4096;
    return 8192;
}

Indexer::Indexer(
        const shared_ptr<InputSource> &fastqfile,
        const path &index,
        int blockInterval,
        bool enableDebugging,
        bool forceOverwrite
) :
        ZLibBasedFASTQProcessorBaseClass(fastqfile, index, enableDebugging),
        blockInterval(blockInterval) {
    indexWriter = make_shared<IndexWriter>(index, forceOverwrite);
}

bool Indexer::checkPremises() {
    return indexWriter->tryOpen();
}

shared_ptr<IndexHeader> Indexer::createHeader() {
    auto header = make_shared<IndexHeader>(Indexer::INDEXER_VERSION, sizeof(IndexEntryV1), blockInterval);
    return header;
}

/**
 * Will use the basic algorithm from https://github.com/madler/zlib/blob/master/examples/zran.c to create an index for a
 * FASTQ file.
 *
 * This method will check, if it was called previously. If so, it will fail, leaving all other variables in their
 * current state. It will add an error message to the error list. It will also fail to run, if there is an active  write
 * lock for the target idx file.
 */
bool Indexer::createIndex() {

    info("Start indexing.");

    if (wasStarted) {
        addErrorMessage("It is not allowed to run Indexer.createIndex() more than once.");
        // finishedSuccessful will not be changed.
        return false;
    }
    wasStarted = true;

    if (!checkPremises()) {
        finishedSuccessful = false;
        return false;
    }

    auto sizeOfFastq = fastqfile->size();
    // If not already set, recalculate the interval for index entries.
    if (blockInterval == -1) {
        if (sizeOfFastq == -1) {
            blockInterval = 512; // Like for 32GB files.
        } else {
            blockInterval = Indexer::calculateIndexBlockInterval(sizeOfFastq);
        }
    }

    // After init, store header, then start indexing
    auto header = createHeader();
    if (enableDebugging)
        storedHeader = header;
    if (!indexWriter->writeIndexHeader(header)) {
        finishedSuccessful = false;
        return false;
    }

    fastqfile->open();

    bool keepProcessing = true;

    if (!initializeZStreamForInflate()) {
        finishedSuccessful = false;
        return false;
    }
    while (keepProcessing) {

        do {
            if (!fastqfile->canRead()) {
                keepProcessing = false; // Happens, when input streams are used.
                break;
            }
            if (!readCompressedDataFromInputSource()) {
                errorWasRaised = true;
                break;
            }
            // Process read data or until end of stream
            do {
                checkAndResetSlidingWindow();

                bool checkForStreamEnd = true;

                if (!decompressNextChunkOfData(checkForStreamEnd, Z_BLOCK)) {
                    // In some cases, the data chunk after the initial block can be so small, that the Indexer will
                    // instantly skip the part, because stream end is reached and NO index entry get written. To prevent
                    // this, we check for stream end and finalize anyway. Then break.
                    if (zlibResult == Z_STREAM_END) {
                        finalizeProcessingForCurrentBlock(currentDecompressedBlock, &zStream);
                    }
                    break;
                }

                if (checkStreamForBlockEnd()) {
                    finalizeProcessingForCurrentBlock(currentDecompressedBlock, &zStream);
                }

                firstPass = false;

            } while (zStream.avail_in != 0);

            keepProcessing = zlibResult != Z_STREAM_END && fastqfile->canRead();

        } while (!errorWasRaised && keepProcessing);

        storeLinesOfCurrentBlockForDebugMode(currentDecompressedBlock);

        /**
         * We also want to process concatenated gzip files.
         */
        if (!keepProcessing) {
            keepProcessing = checkAndPrepareForNextConcatenatedPart();
        }
    }

    fastqfile->close();
    inflateEnd(&zStream);

    finishedSuccessful = !errorWasRaised;
    if (errorWasRaised) {
        addErrorMessage("There were errors during index creation. Index file is corrupt.");
    } else {
        cerr << "Finished indexing with the last entry for compressed block #" << lastStoredEntry->blockID
             << " starting with entry number " << lastStoredEntry->startingLineInEntry << "\n";
        if (numberOfConcatenatedFiles > 1) {
            cerr << " The source data consisted of " << numberOfConcatenatedFiles << " concatenated gzip streams.\n";
        }
    }
    return finishedSuccessful;
}

bool Indexer::checkAndPrepareForNextConcatenatedPart() {
//    bool keepProcessing{false};

    // The stream position needs to be set to the beginning of the new gzip stream. If reading from the data stream is
    // possible afterwards, there might be another gzip stream and we will continue decompression.
    fastqfile->seek(totalBytesIn, true);
    if (!fastqfile->canRead())
        return false;

    // Clean up and go on
    inflateEnd(&zStream);
    if (!initializeZStreamForInflate()) {
        finishedSuccessful = false;
        return false;
    }
    checkAndResetSlidingWindow();
    numberOfConcatenatedFiles++;
    memset(window, 0, WINDOW_SIZE);
    memset(input, 0, CHUNK_SIZE);
    zStream.avail_in = CHUNK_SIZE;
    firstPass = true;

    lastBlockEndedWithNewline = true;

    currentDecompressedBlock.str("");
    currentDecompressedBlock.clear();

    return true;
}

/**
 * Several cases for lines in blocks
 * A:
 * abc\n
 * abc\n
 * abc\n
 * => 3 Lines, each with a newline at the end, counting from 0 to 2 (starts with 0)
 * => lineOffset = 0, noOfLines = 3
 * B:
 * abc\n
 * abc\n
 * ab
 * => 3 Lines, two with a newline at the end, counting from 3 to 5 (starts with 3)
 * => lineOffset = 0, noOfLines = 3
 *
 * C:
 * c\n
 * abc\n
 * ab
 * => 2 Lines, c does not count, one with a newline at the end, counting from 6 to 7 (starts with 6)
 * => lineOffset = 1 (next line so to say), noOfLines = 2
 *
 * D:
 * c\n
 * abc\n
 * abc\n
 * => 2 Lines, each with a newline at the end, counting from 0 to 2 (starts with 0)
 *
 * => lastEndedWithNewline Cases 1, 2
 * => !lastEnded...        Cases 3, 4
 *
 * @param currentDecompressedBlock
 * @param strm
 * @param curWindow
 * @param cleansedWindow
 */
void Indexer::finalizeProcessingForCurrentBlock(stringstream &currentDecompressedBlock, z_stream *strm) {
    u_int64_t blockOffset = offset;     // Store current blockOffsetInRawFile.
    offset = totalBytesIn;          // Set new blockOffsetInRawFile.
    if (!firstPass) {
        blockID++;

        // String representation of currentDecompressedBlock. Might or might not start with a fresh line, we need to
        // figure this out.
        std::vector<string> lines;
        string currentBlockString = currentDecompressedBlock.str();

        lines = splitStr(currentBlockString);

        u_int32_t sizeOfCurrentBlock = currentBlockString.size();
        u_int32_t numberOfLinesInBlock = lines.size();

        // Check the current block and see, if the last character is '\n'. If so, the blockOffsetInRawFile of the first
        // line in the next IndexEntry will be 0. Otherwise, we need to find the blockOffsetInRawFile.
        bool currentBlockEndedWithNewLine =
                sizeOfCurrentBlock == 0 ? true : currentBlockString[sizeOfCurrentBlock - 1] == '\n';

        // Find the first newline character to get the blockOffsetInRawFile of the line inside
        ushort offsetOfFirstLine{0};
        if (currentBlockString.empty()) {
        } else if (!lastBlockEndedWithNewline) {
            numberOfLinesInBlock--;  // If the last block ended with an incomplete line (and not '\n'), reduce this.
            offsetOfFirstLine = lines[0].size() + 1;
        } else if (lineCountForNextIndexEntry > 0) {
            lineCountForNextIndexEntry--;
        }

        // Only store every n'th block.
        // Create the index entry
        auto entry = make_shared<IndexEntryV1>(
                curBits,
                blockID,
                offsetOfFirstLine,
                blockOffset,
                lineCountForNextIndexEntry);

        // Compared to the original zran example, which uses two memcpy operations to retrieve the dictionary, we
        // use zlibs inflateGetDictionaryMethod. This looks more clean and works, whereas I could not get the
        // original memcpy operations to work.
//        Bytef dictTest[WINDOW_SIZE];              // Build in later, around 60% decrease in size.
//        u_int64_t compressedBytes = WINDOW_SIZE;
//        compress2(dictTest, &compressedBytes, dictionaryForNextBlock, WINDOW_SIZE, 9);

        u_int32_t copiedBytes = 0;
        memcpy(entry->dictionary, dictionaryForNextBlock, WINDOW_SIZE);
        inflateGetDictionary(strm, dictionaryForNextBlock, &copiedBytes);

        // Only write back every nth entry. As we need the large window / dictionary of 32kb, we'll need to save
        // some space here. Think of large NovaSeq FASTQ files with ~200GB! We can't store approximately 3.6m index
        // entries which would be around 115GB for an index file.
        if (blockID % blockInterval == 0) {
            indexWriter->writeIndexEntry(entry);
            lastStoredEntry = entry;
            if (enableDebugging) {
                storedEntries.emplace_back(entry);
            }
        }

        storeLinesOfCurrentBlockForDebugMode(currentDecompressedBlock);

        // Keep some info for next entry.
        lastBlockEndedWithNewline = currentBlockEndedWithNewLine;
        lineCountForNextIndexEntry += numberOfLinesInBlock;

        // This will pop up a clang-tidy warning, but as Mark Adler does it, I don't want to change it.
        curBits = strm->data_type & 7;
    }

    currentDecompressedBlock.str("");
    currentDecompressedBlock.clear();
}

void Indexer::storeLinesOfCurrentBlockForDebugMode(std::stringstream &currentDecompressedBlock) {
    if (!enableDebugging) return;

    string str = currentDecompressedBlock.str();
    std::vector<string> lines = splitStr(str);

    if (lines.empty()) return;

    if (!lastBlockEndedWithNewline) {
        // Check this and eventually join the two broken strings.
        auto idx = storedLines.size() - 1;
        lines[0] = storedLines[idx] + lines[0];
        storedLines.pop_back();
    }
    if (lines[lines.size() - 1].empty())
        lines.pop_back();
    storedLines.insert(storedLines.end(), lines.begin(), lines.end());
}

vector<string> Indexer::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexWriter->getErrorMessages();
    return mergeToNewVector(l, r);
}
