/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Indexer.h"
#include "IndexEntryStorageStrategy.h"
#include "runners/ActualRunner.h"
#include "runners/IndexStatsRunner.h"
#include "common/IOHelper.h"
#include "common/StringHelper.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <zlib.h>
#include <experimental/filesystem>

using std::experimental::filesystem::path;

const unsigned int Indexer::INDEXER_VERSION = 1;

Indexer::Indexer(
        const shared_ptr<Source> &fastqfile,
        const shared_ptr<Sink> &index,
        shared_ptr<IndexEntryStorageStrategy> storageStrategy,
        bool enableDebugging,
        bool forceOverwrite,
        bool forbidWriteFQI,
        bool compressDictionaries
) : ZLibBasedFASTQProcessorBaseClass(fastqfile, index, enableDebugging) {
    this->forbidWriteFQI = forbidWriteFQI;
    this->storageStrategy = storageStrategy;
    this->forceOverwrite = forceOverwrite;
    this->compressDictionaries = compressDictionaries;
    if (!forbidWriteFQI)
        indexWriter = make_shared<IndexWriter>(index, forceOverwrite, compressDictionaries);
}

bool Indexer::checkPremises() {
    if (!forbidWriteFQI)
        return indexWriter->tryOpen();
    return true;
}

shared_ptr<IndexHeader> Indexer::createHeader() {
    auto header = make_shared<IndexHeader>(Indexer::INDEXER_VERSION, sizeof(IndexEntryV1), 0,
                                           compressDictionaries);
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

    auto sizeOfFastq = fastqFile->size();
    // If not already set, recalculate the interval for index entries.
    storageStrategy->useFilesizeForCalculation(sizeOfFastq);

    // After init, store header, then start indexing
    auto header = createHeader();
    if (enableDebugging)
        storedHeader = header;

    if (!forbidWriteFQI && !indexWriter->writeIndexHeader(header)) {
        finishedSuccessful = false;
        return false;
    }
    if (forbidWriteFQI) {
        cerr << "The indexer will not write an index file!\n";
    }

    fastqFile->open();

    bool keepProcessing = true;

    if (!initializeZStreamForInflate()) {
        finishedSuccessful = false;
        return false;
    }

    if (writeOutOfPartialDecompressedBlocks) {
        partialBlockinfoStream.open(storageForPartialDecompressedBlocks);
    }

    while (keepProcessing) {

        do {
            if (!fastqFile->canRead()) {
                keepProcessing = false; // Happens, when input streams are used.
                break;
            }
            if (!readCompressedDataFromSource()) {
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

            keepProcessing = zlibResult != Z_STREAM_END && fastqFile->canRead();

        } while (!errorWasRaised && keepProcessing);

        storeLinesOfCurrentBlockForDebugMode(currentDecompressedBlock);

        /**
         * We also want to process concatenated gzip files.
         */
        if (!keepProcessing) {
            keepProcessing = checkAndPrepareForNextConcatenatedPart();
        }
    }

    if (writeOutOfPartialDecompressedBlocks) {
        partialBlockinfoStream.flush();
        partialBlockinfoStream.close();
    }

    fastqFile->close();
    inflateEnd(&zStream);

    // Set line info for index file, which will be written, when the index writer is deleted.
    indexWriter->setNumberOfLinesInFile(this->lineCountForNextIndexEntry);

    finishedSuccessful = !errorWasRaised;
    if (errorWasRaised) {
        addErrorMessage("There were errors during index creation. Index file is corrupt.");
    } else {
        cerr << "Finished indexing with the last entry for compressed block #" << lastStoredEntry->blockID
             << " starting with line number " << lastStoredEntry->startingLineInEntry << "\n"
             << " The indexed file contains " << this->lineCountForNextIndexEntry << " lines\n";
        if (numberOfConcatenatedFiles > 1) {
            cerr << " The source data consisted of " << numberOfConcatenatedFiles << " concatenated gzip streams.\n";
        }
    }

    indexWriter->finalize();
    return finishedSuccessful;
}

bool Indexer::checkAndPrepareForNextConcatenatedPart() {
    // The stream position needs to be set to the beginning of the new gzip stream. If reading from the data stream is
    // possible afterwards, there might be another gzip stream and we will continue decompression.
    fastqFile->seek(totalBytesIn, true);
    if (!fastqFile->canRead())
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

    clearCurrentCompressedBlock();

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
    if (firstPass) {
        clearCurrentCompressedBlock();
        return;
    }

    blockID++;

    // String representation of currentDecompressedBlock. Might or might not start with a fresh line, we need to
    // figure this out.
    u_int32_t numberOfLinesInBlock{0};
    string currentBlockString = currentDecompressedBlock.str();
    std::vector<string> lines = StringHelper::splitStr(currentBlockString);
    bool currentBlockEndedWithNewLine{false};
    bool blockIsEmpty = currentBlockString.empty();

    if (writeOutOfDecompressedBlocksAndStatistics) {
        path outfile =
                storageForDecompressedBlocks.u8string() + string("/") + "decompressedblock_" + to_string(blockID) +
                ".txt";
        ofstream blockStream;
        blockStream.open(outfile);
        blockStream << currentBlockString;
        blockStream.flush();
        blockStream.close();
    }


    shared_ptr<IndexEntryV1> entry = createIndexEntryFromBlockData(
            currentBlockString,
            lines,
            blockOffset,
            lastBlockEndedWithNewline,
            &currentBlockEndedWithNewLine,
            &numberOfLinesInBlock
    );

    storeDictionaryForEntry(strm, entry);

    bool written = writeIndexEntryIfPossible(entry, lines, blockIsEmpty);

    if (writeOutOfPartialDecompressedBlocks) {

        partialBlockinfoStream << "\n---- Block: " << blockID
                               << "\n\tlNL: " << lastBlockEndedWithNewline
                               << "\n\tcNL: " << currentBlockEndedWithNewLine
                               << "\n\t#L:  " << numberOfLinesInBlock
                               << "\n\toff: " << entry->offsetOfFirstValidLine
                               << "\n\tsl:  " << entry->startingLineInEntry
                               << "\n\tsts: " << (storageStrategy->wasPostponed() ? "Postponed" : written ? "Written" : "Skipped")
                               << "\n";
        if (blockIsEmpty) {
            partialBlockinfoStream << "EMPTY BLOCK!\n";
        } else if (currentBlockString.size() > 20) {
            partialBlockinfoStream << "STARTING 20Byte:\n'" << currentBlockString.substr(0, 20);
            partialBlockinfoStream << "'\nENDING 20Byte:\n'"
                                   << currentBlockString.substr(currentBlockString.size() - 20, 20) << "'";
        } else {
            partialBlockinfoStream << "WHOLE BLOCK DATA:\n'" << currentBlockString << "'";
        }
        partialBlockinfoStream.flush();
    }

    storeLinesOfCurrentBlockForDebugMode(currentDecompressedBlock);

    // Keep some info for next entry.
    lastBlockEndedWithNewline = currentBlockEndedWithNewLine;

    // This will pop up a clang-tidy warning, but as Mark Adler does it, I don't want to change it.
    curBits = strm->data_type & 7;

    clearCurrentCompressedBlock();
}

bool Indexer::writeIndexEntryIfPossible(shared_ptr<IndexEntryV1> &entry,
                                        const vector<string> &lines,
                                        bool blockIsEmpty) {

    if(!storageStrategy->shallStore(entry, blockID, blockIsEmpty))
        return false;

    // Now, if we store the entry and dictionary compression is enable, do exactly that!
    if (compressDictionaries) {
        Bytef compressedDictionary[WINDOW_SIZE]{0};              // Around 60% decrease in size.
        u_int64_t compressedBytes = WINDOW_SIZE;
        auto result = compress2(compressedDictionary, &compressedBytes, entry->dictionary, WINDOW_SIZE, 9);
        entry->compressedDictionarySize = (u_int16_t) compressedBytes;
        memset(entry->dictionary, 0, WINDOW_SIZE);
        memcpy(entry->dictionary, compressedDictionary, compressedBytes);
    }

    storageStrategy->resetPostponeWrite();
    if (!forbidWriteFQI)
        indexWriter->writeIndexEntry(entry);
    lastStoredEntry = entry;
    if (enableDebugging) {
        storedEntries.emplace_back(entry);
    }
    return true;
}

void Indexer::storeDictionaryForEntry(z_stream *strm, shared_ptr<IndexEntryV1> entry) {
    // Compared to the original zran example, which uses two memcpy operations to retrieve the dictionary, we
    // use zlibs inflateGetDictionaryMethod. This looks more clean and works, whereas I could not get the
    // original memcpy operations to work.

    u_int32_t copiedBytes = 0;
    memcpy(entry->dictionary, dictionaryForNextBlock, WINDOW_SIZE);
    inflateGetDictionary(strm, dictionaryForNextBlock, &copiedBytes);
}

shared_ptr<IndexEntryV1> Indexer::createIndexEntryFromBlockData(const string &currentBlockString,
                                                                const vector<string> &lines,
                                                                u_int64_t &blockOffsetInRawFile,
                                                                bool lastBlockEndedWithNewline,
                                                                bool *currentBlockEndedWithNewLine,
                                                                u_int32_t *numberOfLinesInBlock) {
    u_int32_t sizeOfCurrentBlock = currentBlockString.size();
    *numberOfLinesInBlock = lines.size();

    // Check the current block and see, if the last character is '\n'. If so, the blockOffsetInRawFile of the first
    // line in the next IndexEntry will be 0. Otherwise, we need to find the blockOffsetInRawFile.
    *currentBlockEndedWithNewLine =
            sizeOfCurrentBlock == 0 ? lastBlockEndedWithNewline : currentBlockString[sizeOfCurrentBlock - 1] == '\n';
    bool hasAnyLineBreaks = false;
    auto currentBlockCString = currentBlockString.c_str();
    for (int i = 0; i < currentBlockString.size(); i++) {
        if (currentBlockCString[i] == '\n') {
            hasAnyLineBreaks = true;
            break;
        }
    }

    // Find the first newline character to get the blockOffsetInRawFile of the line inside
    ushort offsetOfFirstLine{0};
    if (currentBlockString.empty()) {
    } else if (!lastBlockEndedWithNewline) {
        (*numberOfLinesInBlock)--;  // If the last block ended with an incomplete line (and not '\n'), reduce this.
        if (hasAnyLineBreaks)   // See case 3.1 in testlayout. a block with a \n at any position
            offsetOfFirstLine = lines[0].size() + 1;
    }
    //!*currentBlockEndedWithNewLine &&
    // Only store every n'th block.
    // Create the index entry
    auto entry = make_shared<IndexEntryV1>(
            curBits,
            blockID,
            offsetOfFirstLine,
            blockOffsetInRawFile,
            lineCountForNextIndexEntry);

    lineCountForNextIndexEntry += *numberOfLinesInBlock;

    return entry;
}

void Indexer::storeLinesOfCurrentBlockForDebugMode(std::stringstream &currentDecompressedBlock) {
    if (!enableDebugging) return;

    string str = currentDecompressedBlock.str();
    std::vector<string> lines = StringHelper::splitStr(str);

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
    if (!forbidWriteFQI) {
        vector<string> l = ErrorAccumulator::getErrorMessages();
        vector<string> r = indexWriter->getErrorMessages();
        return mergeToNewVector(l, r);
    }
    return ErrorAccumulator::getErrorMessages();
}