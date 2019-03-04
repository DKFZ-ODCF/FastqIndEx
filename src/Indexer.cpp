/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Indexer.h"
#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

const unsigned int Indexer::INDEXER_VERSION = 1;

uint Indexer::calculateIndexBlockInterval(ulong fileSize) {
    const ulong GB = 1024 * 1024 * 1024;
    ulong testSize = fileSize / GB;

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

Indexer::Indexer(const path &fastq, const path &index, int blockInterval, bool enableDebugging) :
        fastq(fastq),
        index(index),
        blockInterval(blockInterval),
        debuggingEnabled(enableDebugging) {
    indexWriter = boost::shared_ptr<IndexWriter>(new IndexWriter(index));
}

Indexer::~Indexer() {
}

bool Indexer::checkPremises() {
    return indexWriter->tryOpen();
}

boost::shared_ptr<IndexHeader> Indexer::createHeader() {
    auto header = boost::make_shared<IndexHeader>(Indexer::INDEXER_VERSION, sizeof(IndexEntryV1), blockInterval);
    return header;
}

/**
 * I would have preferred to initialize this more C++ 11 like but I wanted to stick to the original implementation and
 * do not know enough about the internals of zlib to do it differently.
 */
bool Indexer::initializeZStream(z_stream *const strm) {
    strm->zalloc = nullptr;
    strm->zfree = nullptr;
    strm->opaque = nullptr;
    strm->avail_in = 0;
    strm->next_in = nullptr;

    auto zlibResult = inflateInit2(strm, 47);   /* automatic zlib or gzip decoding */
    if (zlibResult != Z_OK) {
        addErrorMessage("The zlib stream could not be initialized.");
        return false;
    }

    return true;
}

/**
 * (Unfortunately,) zlib uses the C File API. Use that as well to read from the inputFile to the buffer.
 * @param inputFile File to read from (FASTQ)
 * @param strm Reference to the strm struct which is used by zlib decompression.
 * @param buffer which will hold the latest decompressed chunk of data.
 * @return
 */
bool Indexer::readCompressedDataFromStream(FILE *const inputFile, z_stream *const strm, Byte *const buffer) {
    /* get some compressed data from input file */
    strm->avail_in = std::fread((void *) buffer, 1, CHUNK_SIZE, inputFile);
    if (std::ferror(inputFile)) {
        this->addErrorMessage("There was an error during fread.");
        return false;
    }
    if (strm->avail_in == 0) {
        this->addErrorMessage("There was no data available in the stream");
        return false;
    }
    strm->next_in = buffer;
    return true;
}

Bytef dictForNext[32768]{0};

/**
 * Will use the basic algorithm from https://github.com/madler/zlib/blob/master/examples/zran.c to create an index for a
 * FASTQ file.
 *
 * This method will check, if it was called previously. If so, it will fail, leaving all other variables in their
 * current state. It will add an error message to the error list. It will also fail to run, if there is an active  write
 * lock for the target idx file.
 */
bool Indexer::createIndex() {

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

    z_stream zStream;

    if (!initializeZStream(&zStream)) {
        finishedSuccessful = false;
        return false;
    }

    // If not already set, recalculate the interval for index entries.
    if (blockInterval == -1) {
        blockInterval = Indexer::calculateIndexBlockInterval(file_size(this->getFastq()));
    }

    // After init, store header, then start indexing
    auto header = createHeader();
    if (debuggingEnabled)
        storedHeader = header;
    if (!indexWriter->writeIndexHeader(header)) {
        finishedSuccessful = false;
        return false;
    }

    memset(dictForNext, 0, WINDOW_SIZE);

    // Create a C FILE to read from it. We will skip further file checks as they were already performed in earlier steps
    FILE *fastqFile = std::fopen(this->getFastq().c_str(), "rb");

    // Input buffer which holds decompressed data.
    Byte input[CHUNK_SIZE];

    // Sliding window which holds compressed data.
    Byte window[WINDOW_SIZE];
    memset(window, 0, WINDOW_SIZE);

    Byte lastWindow[WINDOW_SIZE];

    // Shows, if an error happened during the index build.
    bool buildIndexError = false;

    // Stores results of zlib operations.
    int zlibResult{0};

    // Cannot be moved to class level, as the operator= seems to be deleted.
    // Keeps the decompressed string content of the currently processed block.
    stringstream currentDecompressedBlock;
    zStream.avail_out = 0;
    do {
//        ulong blockBegin = ftell()
        if (!readCompressedDataFromStream(fastqFile, &zStream, input)) {
            buildIndexError = true;
            break;
        }
        // Process read data or until end of stream
        do {
            // Reset sliding window
            if (zStream.avail_out == 0) {
                zStream.avail_out = WINDOW_SIZE;
                zStream.next_out = window;
            }
//            memset(window, 0, WINDOW_SIZE);

            // Inflate until out of input, output, or at end of block
            // Update the total input and output counters
            ulong availableInBeforeInflate = zStream.avail_in;
            ulong availableOutBeforeInflate = zStream.avail_out;
            uint windowPositionBeforeInflate = WINDOW_SIZE - zStream.avail_out;

            zlibResult = inflate(&zStream, Z_BLOCK);

            ulong readBytes = availableInBeforeInflate - zStream.avail_in;
            ulong writtenBytes = availableOutBeforeInflate - zStream.avail_out;

            totalBytesIn += readBytes;
            totalBytesOut += writtenBytes;

            Bytef cleansedWindowForCout[CLEAN_WINDOW_SIZE];
            memset(cleansedWindowForCout, 0, CLEAN_WINDOW_SIZE);

            std::memcpy(cleansedWindowForCout, window + windowPositionBeforeInflate, writtenBytes);

            if (zlibResult == Z_NEED_DICT) {
                zlibResult = Z_DATA_ERROR;
            }
            if (zlibResult == Z_MEM_ERROR || zlibResult == Z_DATA_ERROR) {
                buildIndexError = true;
                break;
            }
            if (zlibResult == Z_STREAM_END) {
                break;
            }

            currentDecompressedBlock << cleansedWindowForCout;

            if (checkStreamForBlockEnd(&zStream)) {
                finalizeProcessingForCurrentBlock(currentDecompressedBlock, &zStream);
            }

            firstBlock = false;

        } while (zStream.avail_in != 0);

    } while (!buildIndexError && zlibResult != Z_STREAM_END);

    if (debuggingEnabled) {
        storeLinesOfCurrentBlockForDebugMode(currentDecompressedBlock);
    }

    // Free the file pointer and close the file.
    fclose(fastqFile);
    inflateEnd(&zStream);

    if (buildIndexError) {
        addErrorMessage("There were errors during index creation. Index file is corrupt.");
        finishedSuccessful = false;
    } else {
        finishedSuccessful = true;
    }
    return finishedSuccessful;
}

/**
 * If at end of block, consider adding an index entry (note that if
 * data_type indicates an end-of-block, then all of the
 * uncompressed data from that block has been delivered, and none
 * of the compressed data after that block has been consumed,
 * except for up to seven bits) -- the total_bytes_out == 0 provides an
 * entry point after the zlib or gzip header, and assures that the
 * index always has at least one access point; we avoid creating an
 * access point after the offset block by checking bit 6 of data_type
 *
 * @param strm z_stream reference to the struct instance used for decompression
 * @return true, if at end of block.
 */
bool Indexer::checkStreamForBlockEnd(z_stream *strm) const {
    return (strm->data_type & 128) != 0 && !(strm->data_type & 64) != 0;
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
    ulong blockOffset = offset;     // Store current blockOffsetInRawFile.
    offset = totalBytesIn;          // Set new blockOffsetInRawFile.
    if (!firstBlock) {
        blockID++;
        // String representation of currentDecompressedBlock. Might or might not start with a fresh line, we need to
        // figure this out.
        string currentBlockString = currentDecompressedBlock.str();
        uint sizeOfCurrentBlock = currentBlockString.size();
        uint numberOfLinesInBlock;

        std::vector<string> lines;
        string str = currentDecompressedBlock.str();
        boost::split(lines, str, boost::is_any_of("\n"));
        numberOfLinesInBlock = lines.size();

        // Check the current block and see, if the last character is '\n'. If so, the blockOffsetInRawFile of the first line in the
        // next IndexEntry will be 0. Otherwise, we need to find the blockOffsetInRawFile.
        bool currentBlockEndedWithNewLine =
                sizeOfCurrentBlock == 0 ? true : currentBlockString[sizeOfCurrentBlock - 1] == '\n';

        string firstLine = lines[0];

        // Find the first newline character to get the blockOffsetInRawFile of the line inside
        ushort offsetOfFirstLine{0};
        if (!lastBlockEndedWithNewline) {
            firstLine = lines[1];
            const char *curBlockCStr = currentBlockString.c_str();
            numberOfLinesInBlock--;  // If the last block ended with an incomplete line (and not '\n'), reduce this.
            for (int i = 0; i < sizeOfCurrentBlock && curBlockCStr[i] != 0; i++) {
                if (curBlockCStr[i] == '\n') {
                    offsetOfFirstLine = i + 1;
                    break;  // We can skip the rest, we found the newline.
                }
            }
        } else if (totalLineCount > 0) {
            totalLineCount--;
        }

        // Only store every n'th block.
        // Create the index entry
        auto entry = boost::make_shared<IndexEntryV1>(
                curBits,
                blockID,
                offsetOfFirstLine,
                blockOffset,
                totalLineCount);

        // Compared to the original zran example, which uses two memcpy operations to retrieve the dictionary, we
        // use zlibs inflateGetDictionaryMethod. This looks more clean and works, whereas I could not get the
        // original memcpy operations to work.
            Bytef dictTest[WINDOW_SIZE];              // Build in later, around 60% decrease in size.
            uLong compressedBytes = WINDOW_SIZE;
            compress2(dictTest, &compressedBytes, dictForNext, WINDOW_SIZE, 9);

        uInt copiedBytes = 0;
        memcpy(entry->dictionary, dictForNext, WINDOW_SIZE);
        inflateGetDictionary(strm, dictForNext, &copiedBytes);

        // Only write back every nth entry. As we need the large window / dictionary of 32kb, we'll need to save
        // some space here. Think of large NovaSeq FASTQ files with ~200GB! We can't store approximately 3.6m index
        // entries which would be around 115GB for an index file.
        if (blockID % blockInterval == 0) {
            indexWriter->writeIndexEntry(entry);
            if (debuggingEnabled) {
                storedEntries.emplace_back(entry);
            }
        }

        if (debuggingEnabled) {
            storeLinesOfCurrentBlockForDebugMode(currentDecompressedBlock);
        }

        // Keep some info for next entry.
        lastBlockEndedWithNewline = currentBlockEndedWithNewLine;
        totalLineCount += numberOfLinesInBlock;
        curBits = strm->data_type & 7;
    }

    currentDecompressedBlock.str("");
    currentDecompressedBlock.clear();
}

void Indexer::storeLinesOfCurrentBlockForDebugMode(std::stringstream &currentDecompressedBlock) {
    std::vector<string> lines;
    string str = currentDecompressedBlock.str();
    boost::split(lines, str, boost::is_any_of("\n"));

    if (lines.size() == 0) return;

    if (!lastBlockEndedWithNewline) {
        // Check this and eventually join the two broken strings.
        auto idx = storedLines.size() - 1;
        lines[0] = storedLines[idx] + lines[0];
        storedLines.pop_back();
    }
    if (lines[lines.size() - 1] == "")
        lines.pop_back();
    storedLines.insert(storedLines.end(), lines.begin(), lines.end());
}

vector<string> Indexer::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexWriter->getErrorMessages();
    return mergeToNewVector(l, r);

}
