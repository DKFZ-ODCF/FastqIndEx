/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Extractor.h"
#include "ZLibBasedFASTQProcessorBaseClass.h"
#include "PathInputSource.h"
#include <cstdio>
#include <experimental/filesystem>
#include <iostream>
#include <unistd.h>
#include <zlib.h>

using namespace experimental::filesystem;
using namespace std;
using experimental::filesystem::path;

Extractor::Extractor(
        const shared_ptr<PathInputSource> &fastqfile,
        const path &indexfile,
        const path &resultfile,
        bool forceOverwrite,
        u_int64_t startingLine,
        u_int64_t lineCount,
        bool enableDebugging)
        : ZLibBasedFASTQProcessorBaseClass(fastqfile, indexfile, enableDebugging),
          startingLine(startingLine),
          lineCount(lineCount) {
    this->indexReader = make_shared<IndexReader>(indexfile);
    this->resultFile = resultfile;
    this->forceOverwrite = forceOverwrite;

    if (resultFile.empty() || resultFile.generic_u8string() == "-") {
        this->forceOverwrite = false;
    } else {
        char buf[WINDOW_SIZE]{0};
        realpath(this->resultFile.string().c_str(), buf);
        this->resultFile = path(string(buf));
        useFile = true;
    }
}

bool Extractor::checkPremises() {
    if (lineCount == 0) {
        addErrorMessage("Can't extract a line count of 0 lines. The value needs to be a positive number.");
        return false;
    }
    if (useFile) {
        if (!forceOverwrite && exists(resultFile)) {
            addErrorMessage(
                    "The result file already exists and cannot be overwritten. Allow overwriting with -w, if it is intentional.");
            return false;
        }
        if (exists(resultFile) && access(resultFile.string().c_str(), W_OK) != 0) {
            addErrorMessage("The result file exists and cannot be overwritten. Check its file access rights.");
            return false;
        }
        if (!exists(resultFile) && access(resultFile.parent_path().string().c_str(), W_OK) != 0) {
            addErrorMessage("The result file cannot be written. Check the access rights of the parent folder.");
            return false;
        }
    }
    return this->indexReader->tryOpenAndReadHeader();
}

bool Extractor::extractReadsToCout() {
    auto resultFileStream = shared_ptr<ofstream>(nullptr);
    ofstream outfilestream;
    ostream *out;
    if (!useFile)
        out = &cout;
    else {
        outfilestream.open(resultFile.string());
        out = &outfilestream;
    }

    shared_ptr<IndexEntry> previousEntry = indexReader->readIndexEntry();
    shared_ptr<IndexEntry> startingIndexLine = previousEntry;
    while (indexReader->getIndicesLeft() > 0) {
        auto entry = indexReader->readIndexEntry();
        if (entry->startingLineInEntry > startingLine) {
            break;
        }
        startingIndexLine = previousEntry;
    }

    if (!initializeZStreamForRawInflate()) {
        return false;
    }

    fastqfile->open();
    off_t initialOffset = startingIndexLine->offsetInRawFile;
    totalBytesIn += initialOffset;
    int startBits = startingIndexLine->bits;
    if (startBits > 0)
        initialOffset--;
    zlibResult = fastqfile->seek(initialOffset, true);
    if (zlibResult == -1) {
        addErrorMessage("");
        fastqfile->close();
        return false;
    }

    if (startBits > 0) {
        int ret = fastqfile->readChar();
        totalBytesIn++;
        if (ret == -1) {
            ret = fastqfile->lastError() ? Z_ERRNO : Z_DATA_ERROR;
            errorWasRaised = true;
        }
        // This will pop up a clang-tidy warning, but as Mark Adler does it, I don't want to change it.
        zlibResult = inflatePrime(&zStream, startBits, ret >> (8 - startBits));
    }
    zlibResult = inflateSetDictionary(&zStream, startingIndexLine->window, WINDOW_SIZE);

    if (errorWasRaised) {
        addErrorMessage(string("zlib reported an error: ") + zStream.msg);
        fastqfile->close();
        return false;
    }

    // If a block starts with an offset, this can be used to complete the unfinished line of the last block (if necessary)
    string unfinishedLineInLastBlock;

    string incompleteLastLine;

    u_int64_t extractedLines = 0;

    // The number of lines which will be skipped in the found starting block
    u_int64_t skip = startingLine - startingIndexLine->startingLineInEntry;

    // Keep track of all split lines. Merely for debugging
    u_int64_t totalSplitCount = 0;

    bool keepExtracting{false};
    bool finalAbort = false;
    do {
        do {

            if (!readCompressedDataFromInputSource()) {
                errorWasRaised = true;
                break;
            }

            // Process read data or until end of stream
            do {
                checkAndResetSlidingWindow();

                bool checkForStreamEnd = false;

                currentDecompressedBlock.str("");
                currentDecompressedBlock.clear();

                if (!decompressNextChunkOfData(checkForStreamEnd, Z_NO_FLUSH))
                    break;

                std::vector<string> splitLines;
                string str = currentDecompressedBlock.str();
                splitLines = splitStr(str);
                totalSplitCount += splitLines.size();

                string curIncompleteLastLine;
                // Strip away incomplete line, store this line for the next block.
                string lastSplitLine = splitLines[splitLines.size() - 1];
                char lastChar = lastSplitLine.c_str()[lastSplitLine.size() - 1];
                if (lastChar != '\n') {
                    curIncompleteLastLine = lastSplitLine;
                    splitLines.pop_back();
                    totalSplitCount--;
                }

                if (splitLines.empty()) {
                    incompleteLastLine = incompleteLastLine + curIncompleteLastLine;
                    continue;
                }

                // Two options. Extraction began earlier OR we are processing another chunk of data.
                u_int64_t iStart = 0;
                if (extractedLines == 0) {
                    if (startingIndexLine->offsetOfFirstValidLine > 0 && firstPass) {
                        if (!splitLines.empty()) splitLines.erase(splitLines.begin());

                    }
                    iStart = skip;
                } else {
                    if (!incompleteLastLine.empty() && extractedLines < lineCount) {
                        string line = incompleteLastLine + splitLines[0];
                        if (enableDebugging)
                            storedLines.emplace_back(line);
                        else
                            (*out) << line << "\n";
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
                        (*out) << splitLines[i] << "\n";
                    extractedLines++;
                }
                incompleteLastLine = curIncompleteLastLine;
                if (skip > 0) skip -= min(splitLines.size(), skip);

            } while (zStream.avail_in != 0 && zlibResult != Z_STREAM_END);
            finalAbort = !(extractedLines < lineCount && !errorWasRaised);
        } while (!finalAbort && zlibResult != Z_STREAM_END);

        if (enableDebugging) {
            while (storedLines.size() > lineCount) {
                storedLines.pop_back();
            }
        }

        keepExtracting = false;
        if (!finalAbort) {
            totalBytesIn += 8 + 10; // Plus 8 Byte (for what? they are missing...) and 10 Byte for the next header
            uint64_t streamEndPosition = totalBytesIn;
            uintmax_t fileSize = fastqfile->size();
            if (streamEndPosition < fileSize) { // plus 10 Bytes for the next header block
                keepExtracting = true;
                initializeZStreamForRawInflate();
                checkAndResetSlidingWindow();
                memset(this->window, 0, WINDOW_SIZE);
                memset(this->input, 0, CHUNK_SIZE);
                this->zStream.avail_in = CHUNK_SIZE;
                firstPass = true;
                fastqfile->seek(streamEndPosition, true);
                Bytef dict[WINDOW_SIZE]{0};
                zlibResult = inflateSetDictionary(&zStream, dict, WINDOW_SIZE);
            }
        }
    } while (keepExtracting);

    // Free the file pointer and close the file.
    fastqfile->close();
    if (outfilestream.is_open())
        outfilestream.close();
    inflateEnd(&zStream);

    if (errorWasRaised) {
        addErrorMessage(string("Last error message from zlib: ") + zStream.msg);
        finishedSuccessful = false;
    } else {
        finishedSuccessful = true;
    }
    return finishedSuccessful;
}

vector<string> Extractor::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexReader->getErrorMessages();
    return mergeToNewVector(l, r);
}
