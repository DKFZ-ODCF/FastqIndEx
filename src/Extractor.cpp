/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Extractor.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <stdio.h>
#include <zlib.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;

Extractor::Extractor(path fastqfile, path indexfile, ulong startingLine, ulong lineCount, bool enableDebugging) {
    this->fastqfile = fastqfile;
    this->indexfile = indexfile;
    this->enableDebugging = enableDebugging;
    this->indexReader = boost::make_shared<IndexReader>(indexfile);
    this->startingLine = startingLine;
    this->lineCount = lineCount;
}

Extractor::~Extractor() {}

bool Extractor::checkPremises() {
    if (lineCount == 0) {
        addErrorMessage("Can't extract a line count of 0 lines. The value needs to be a positive number.");
        return false;
    }
    return this->indexReader->tryOpenAndReadHeader();
}

bool Extractor::initializeZStream(z_stream *const strm) {
    strm->zalloc = nullptr;
    strm->zfree = nullptr;
    strm->opaque = nullptr;
    strm->next_in = nullptr;
    strm->avail_in = 0;

    auto zlibResult = inflateInit2(strm, -15);   /* raw inflate */
    if (zlibResult != Z_OK) {
        addErrorMessage(string("The zlib stream could not be initialized: ") + zStream.msg);
        return false;
    }

    return true;
}

bool Extractor::readCompressedDataFromStream(FILE *const inputFile, z_stream *const strm, Byte *const buffer) {
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

bool Extractor::extractReadsToCout() {
    vector<IndexLine> indexLines = indexReader->readIndexFile();
    if (indexLines.size() == 0) {
        addErrorMessage("Could not start to extract reads from file.");
    }

    // Look up the proper IndexLine entries.
    IndexLine startingIndexLine;
    for (auto line : boost::adaptors::reverse(indexLines)) {
        if (line.startingLineInEntry <= startingLine) {
            startingIndexLine = line;
            break;
        }
    }

    z_stream zStream;
    if (!initializeZStream(&zStream)) {
        return false;
    }

    FILE *fastqFile = fopen(fastqfile.c_str(), "rb");
    off_t offset = startingIndexLine.offsetInRawFile;
    int startBits = startingIndexLine.bits;
    if (startBits > 0)
        offset--;
    int zlibResult = fseeko(fastqFile, offset, SEEK_SET);
    if (zlibResult == -1) {
        addErrorMessage("");
        fclose(fastqFile);
        return false;
    }

    bool error{false};

    if (startBits > 0) {
        int ret = getc(fastqFile);
        if (ret == -1) {
            ret = ferror(fastqFile) ? Z_ERRNO : Z_DATA_ERROR;
            error = true;
        }
        zlibResult = inflatePrime(&zStream, startBits, ret >> (8 - startBits));
    }
    zlibResult = inflateSetDictionary(&zStream, startingIndexLine.window, WINDOW_SIZE);

    if (error) {
        addErrorMessage(string("zlib reported an error: ") + zStream.msg);
        fclose(fastqFile);
        return false;
    }

    // If a block starts with an offset, this can be used to complete the unfinished line of the last block (if necessary)
    string unfinishedLineInLastBlock("");

    string incompleteLastLine;

    ulong extractedLines = 0;

    // Input buffer which holds compressed data.
    Byte input[CHUNK_SIZE];
    memset(input, 0, CHUNK_SIZE);

    // Sliding window which holds decompressed data.
    Byte window[WINDOW_SIZE];
    memset(window, 0, WINDOW_SIZE);

    // The number of lines which will be skipped in the found starting block
    ulong skip = startingLine - startingIndexLine.startingLineInEntry;

    // Keep track of all split lines. Merely for debugging
    ulong totalSplitCount = 0;

    bool firstPass = true;

    do {

        if (!readCompressedDataFromStream(fastqFile, &zStream, input)) {
            error = true;
            break;
        }

        // Process read data or until end of stream
        do {
            // Reset sliding window
            if (zStream.avail_out == 0) {
                zStream.avail_out = WINDOW_SIZE;
                zStream.next_out = window;
                memset(window, 0, WINDOW_SIZE);
            }

            // Inflate until out of input, output, or at end of block
            // Update the total input and output counters
            ulong availableInBeforeInflate = zStream.avail_in;
            uint windowPositionBeforeInflate = WINDOW_SIZE - zStream.avail_out;

            zlibResult = inflate(&zStream, Z_NO_FLUSH);
            ulong readBytes = availableInBeforeInflate - zStream.avail_in;
            ulong writtenBytes = WINDOW_SIZE - zStream.avail_out - windowPositionBeforeInflate;

            // The window buffer used by inflate will be filled at somehwere between 0 <= n <= WINDOW_SIZE
            // as we work with a string append method, we need to copy the read data to a fresh buffer first.
            Bytef cleansedWindowForCout[WINDOW_SIZE + 1]; // +1 for a definitely 0-terminated c-string!
            memset(cleansedWindowForCout, 0, WINDOW_SIZE + 1);
            std::memcpy(cleansedWindowForCout, window + windowPositionBeforeInflate, writtenBytes);

            totalBytesIn += readBytes;
            totalBytesOut += writtenBytes;

            if (zlibResult == Z_NEED_DICT) {
                zlibResult = Z_DATA_ERROR;
            }
            if (zlibResult == Z_MEM_ERROR || zlibResult == Z_DATA_ERROR) {
                error = true;
                break;
            }

            stringstream currentDecompressedBlock;
            currentDecompressedBlock << cleansedWindowForCout;

            std::vector<string> splitLines;
            string str = currentDecompressedBlock.str();
            boost::split(splitLines, str, boost::is_any_of("\n"));

            totalSplitCount += splitLines.size();

            string curIncompleteLastLine;
            // Strip away incomplete line, store this line for the next block.
            string lastSplitLine = splitLines[splitLines.size() - 1];
            if (!boost::ends_with(lastSplitLine, "\n")) {
                curIncompleteLastLine = lastSplitLine;
                splitLines.pop_back();
                totalSplitCount--;
            }

            if (splitLines.size() == 0) {
                incompleteLastLine = incompleteLastLine + curIncompleteLastLine;
                continue;
            }

            // Two options. Extraction began earlier OR we are processing another chunk of data.
            ulong iStart = 0;
            if (extractedLines == 0) {
                if (startingIndexLine.offsetOfFirstValidLine > 0 && firstPass) {
                    if (splitLines.size() > 0) splitLines.erase(splitLines.begin());

                }
                iStart = skip;
            } else {
                if (incompleteLastLine.size() > 0 && extractedLines < lineCount) {
                    string line = incompleteLastLine + splitLines[0];
                    if (enableDebugging)
                        storedLines.emplace_back(line);
                    else
                        cout << line << "\n";
                    extractedLines++;
                    iStart = 1;
                }
            }
            // Tell the extractor, that the inner loop was called at least once, so we don't remove the first line of
            // the splitLines on every iteration.
            firstPass = false;

            // iStart is 0 or 1
            for (int i = iStart; i < splitLines.size() && extractedLines < lineCount; ++i) {
                if (enableDebugging)
                    storedLines.emplace_back(splitLines[i]);
                else
                    cout << splitLines[i] << "\n";
                extractedLines++;
            }
            incompleteLastLine = curIncompleteLastLine;
            if (skip > 0) skip -= min(splitLines.size(), skip);

        } while (zStream.avail_in != 0 && zlibResult != Z_STREAM_END);
    } while (extractedLines < lineCount && !error && zlibResult != Z_STREAM_END);

    if (error) {
        addErrorMessage(string("Last error message from zlib: ") + zStream.msg);
        fclose(fastqFile);
        return false;
    }

    if (enableDebugging) {
        while (storedLines.size() > lineCount) {
            storedLines.pop_back();
        }
    }

    fclose(fastqFile);
    return true;
}