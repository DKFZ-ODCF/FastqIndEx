/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Indexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <boost/algorithm/string.hpp>

#include <zlib.h>

const unsigned int Indexer::CHUNK_SIZE = 16 * 1024;

const unsigned int Indexer::WINDOW_SIZE = 32 * 1024;

const unsigned int Indexer::INDEXER_VERSION = 1;

Indexer::Indexer(const path &fastq, const path &index, bool enableDebugging) :
        fastq(fastq),
        index(index),
        debuggingEnabled(enableDebugging) {
    indexWriter = boost::shared_ptr<IndexWriter>(new IndexWriter(index));
}

bool Indexer::checkPremises() {
    return indexWriter->tryOpen();
}

boost::shared_ptr<IndexHeader> Indexer::createHeader() {
    return boost::make_shared<IndexHeader>(Indexer::INDEXER_VERSION, sizeof(IndexEntryV1));
}

/**
 * I would have preferred to initialize this more C++ 11 like but I wanted to stick to the original implementation and
 * do not know enough about the internals of zlib to do it differently.
 */
bool Indexer::initializeZStream(z_stream *const strm) {
    strm->zalloc = nullptr;
    strm->zfree = nullptr;
    strm->opaque = nullptr;
    strm->next_in = nullptr;
    strm->avail_in = 0;

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


    if (!initializeZStream(&zStream)) {
        finishedSuccessful = false;
        return false;
    }

    // After init, store header, then start indexing
    auto header = createHeader();
    if (debuggingEnabled)
        storedHeader = header;
    if (!indexWriter->writeIndexHeader(header)) {
        finishedSuccessful = false;
        return false;
    };

    // Create a C FILE to read from it. We will skip further file checks as they were already performed in earlier steps
    FILE *fastqFile = std::fopen(this->getFastq().c_str(), "rb");

    // Input buffer which holds decompressed data.
    Byte input[CHUNK_SIZE]{0};

    // Sliding window which holds compressed data.
    Byte window[WINDOW_SIZE]{0};

    // Shows, if an error happened during the index build.
    bool buildIndexError = false;

    // Stores results of zlib operations.
    int zlibResult{0};

    // Cannot be moved to class level, as the operator= seems to be deleted.
    // Keeps the decompressed string content of the currently processed block.
    stringstream currentDecompressedBlock;

    do {
        if (!readCompressedDataFromStream(fastqFile, &zStream, input)) {
            buildIndexError = true;
            break;
        }

        // Process read data or until end of stream
        do {
            // Reset sliding window
            zStream.avail_out = WINDOW_SIZE;
            zStream.next_out = window;
            memset(window, 0, WINDOW_SIZE);

            // Inflate until out of input, output, or at end of block
            // Update the total input and output counters
            ulong availableInBeforeInflate = zStream.avail_in;

            zlibResult = inflate(&zStream, Z_BLOCK);

            ulong readBytes = availableInBeforeInflate - zStream.avail_in;
            ulong writtenBytes = WINDOW_SIZE - zStream.avail_out;

            totalBytesIn += readBytes;
            totalBytesOut += writtenBytes;

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

            currentDecompressedBlock << window;

            if (checkStreamForBlockEnd(zStream)) {
                finalizeProcessingForCurrentBlock(currentDecompressedBlock);
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
bool Indexer::checkStreamForBlockEnd(const z_stream &strm) const {
    return (strm.data_type & 128) != 0 && !(strm.data_type & 64) != 0;
}

void Indexer::finalizeProcessingForCurrentBlock(stringstream &currentDecompressedBlock) {
//    if (debuggingEnabled) {
//        cout << "Processing block " << indexCounter << (firstBlock ? ", first block!": "") << "\n";
//    }
    ulong blockOffset = offset;     // Store current relativeBlockOffsetInRawFile.
    offset = totalBytesIn;          // Set new relativeBlockOffsetInRawFile.
    if (!firstBlock) {
        // String representation of currentDecompressedBlock. Might or migh not start with a fresh line, we need to
        // figure this out.
        string currentBlockString = currentDecompressedBlock.str();
        uint sizeOfCurrentBlock = currentBlockString.size();
        uint numberOfLinesInBlock = count(currentBlockString.begin(), currentBlockString.end(), '\n');

        // Check the current block and see, if the last character is '\n'. If so, the relativeBlockOffsetInRawFile of the first line in the
        // next IndexEntry will be 0. Otherwise, we need to find the relativeBlockOffsetInRawFile.
        bool currentBlockEndedWithNewLine =
                sizeOfCurrentBlock == 0 ? true : currentBlockString[sizeOfCurrentBlock - 1] == '\n';

        // Find the first newline character to get the relativeBlockOffsetInRawFile of the line inside
        ushort offsetOfFirstLine{0};
        if (!lastBlockEndedWithNewline) {
            const char *curBlockCStr = currentBlockString.c_str();
            numberOfLinesInBlock--;  // If the last block ended with an incomplete line (and not '\n'), reduce this.
            for (int i = 0; i < sizeOfCurrentBlock && curBlockCStr[i] != 0; i++) {
                if (curBlockCStr[i] == '\n') {
                    offsetOfFirstLine = i + 1;
                    break;  // We can skip the rest, we found the newline.
                }
            }
        }

        // Create the index entry
        IndexEntryV1();
        auto entry = make_shared<IndexEntryV1>(
                zStream.data_type & 7,
                offsetOfFirstLine,
                blockOffset - lastBlockOffset,
                totalLineCount - startingLineOfLastBlock);
        indexWriter->writeIndexEntry(entry);

        if (debuggingEnabled) {
            storedEntries.emplace_back(entry);
            storeLinesOfCurrentBlockForDebugMode(currentDecompressedBlock);
        }

        // Increment entry index, keep some info for next entry.
        indexCounter++;
        lastBlockEndedWithNewline = currentBlockEndedWithNewLine;
        lastBlockOffset = blockOffset;
        startingLineOfLastBlock = totalLineCount;
        totalLineCount += numberOfLinesInBlock;
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
    return mergeToNewVector(ErrorAccumulator::getErrorMessages(), indexWriter->getErrorMessages());
}
