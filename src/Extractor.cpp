/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Extractor.h"
#include "ZLibBasedFASTQProcessorBaseClass.h"
#include "PathInputSource.h"
#include "IndexStatsRunner.h"
#include <chrono>
#include <cstdio>
#include <experimental/filesystem>
#include <iostream>
#include <unistd.h>
#include <zlib.h>

using namespace experimental::filesystem;
using namespace std;
using namespace std::chrono;

using experimental::filesystem::path;

Extractor::Extractor(
        const shared_ptr<PathInputSource> &fastqfile,
        const path &indexfile,
        const path &resultfile,
        bool forceOverwrite,
        u_int64_t startingLine,
        u_int64_t lineCount,
        uint extractionMulitplier,
        bool enableDebugging
) : ZLibBasedFASTQProcessorBaseClass(fastqfile, indexfile, enableDebugging),
    startingLine(startingLine),
    lineCount(lineCount) {
    this->indexReader = make_shared<IndexReader>(indexfile);
    this->resultFile = resultfile;
    this->forceOverwrite = forceOverwrite;
    this->extractionMulitplier = extractionMulitplier == 0 ? 4 : extractionMulitplier;

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

high_resolution_clock::time_point measurement;

void timerStart() {
    measurement = high_resolution_clock::now();
}

void timerStop(const string &message) {
    auto result = duration_cast<microseconds>(high_resolution_clock::now() - measurement).count();
//    cerr << "Measurement for: " << message << " took " << (result ) << "µs\n";
}

void timerRestart(const string &message) {
    timerStop(message);
    timerStart();
}

bool Extractor::extract() {
    auto resultFileStream = shared_ptr<ofstream>(nullptr);
    ofstream outfilestream;
    ostream *out;
    if (!useFile)
        out = &cout;
    else {
        outfilestream.open(resultFile.string());
        out = &outfilestream;
    }

//    if (startingLine >= extractionMulitplier) {
//        startingLine -= extractionMulitplier;
//        skipLines = extractionMulitplier;
//        lineCount += extractionMulitplier;
//    }

    timerStart();

    shared_ptr<IndexEntry> previousEntry = indexReader->readIndexEntry();
    shared_ptr<IndexEntry> startingIndexLine = previousEntry;

    u_int64_t indexEntryNumber = 0;
    while (indexReader->getIndicesLeft() > 0) {
        auto entry = indexReader->readIndexEntry();
        indexEntryNumber++;
        if (entry->startingLineInEntry > startingLine) {
            break;
        }
        startingIndexLine = entry;
//        previousEntry = entry;
    }

    timerRestart("Index entry search");

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

    timerRestart("Final extractor init");

    IndexStatsRunner::printIndexEntryToConsole(startingIndexLine, indexEntryNumber);

    // The number of lines which will be skipped in the found starting block
    skip = startingLine - startingIndexLine->startingLineInEntry;

    bool keepExtracting = false;
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

                clearCurrentCompressedBlock();

                if (!decompressNextChunkOfData(checkForStreamEnd, Z_NO_FLUSH))
                    break;

                if (!processDecompressedData(out, currentDecompressedBlock.str(), startingIndexLine))
                    continue;

                // Tell the extractor, that the inner loop was called at least once, so we don't remove the first line of
                // the splitLines on every iteration.
                firstPass = false;

            } while (zStream.avail_in != 0 && zlibResult != Z_STREAM_END);
            finalAbort = !(extractedLines < lineCount && !errorWasRaised);
        } while (!finalAbort && zlibResult != Z_STREAM_END);

//        if (!incompleteLastLine.empty()) {
//            cerr << "Warning: The file you extracted from does not end with a newline! Please check the source data.\n";
//            storeOrOutputLine(out, &skipLines, incompleteLastLine);
//        }

        storeLinesOfCurrentBlockForDebugMode();

        keepExtracting = checkAndPrepareForNextConcatenatedPart(finalAbort);
    } while (keepExtracting);

    // Free the file pointer and close the file.
    fastqfile->close();

    if (outfilestream.is_open())
        outfilestream.close();

    inflateEnd(&zStream);

    if (errorWasRaised)
        addErrorMessage(string("Last error message from zlib: ") + zStream.msg);

    return !errorWasRaised;
}

bool Extractor::checkAndPrepareForNextConcatenatedPart(bool finalAbort) {
    if (finalAbort) return false;

    totalBytesIn += 8 + 10; // Plus 8 Byte (for what? they are missing...) and 10 Byte for the next header
    uint64_t streamEndPosition = totalBytesIn;
    fastqfile->seek(streamEndPosition, true);

    if (!fastqfile->canRead()) return false;

    inflateEnd(&zStream);
    if (!initializeZStreamForRawInflate()) {
        finishedSuccessful = false;
        return false;
    }

    checkAndResetSlidingWindow();
    memset(window, 0, WINDOW_SIZE);
    memset(input, 0, CHUNK_SIZE);
    zStream.avail_in = CHUNK_SIZE;
    firstPass = true;

    Bytef dict[WINDOW_SIZE]{0};
    zlibResult = inflateSetDictionary(&zStream, dict, WINDOW_SIZE);

    return true;
}

bool Extractor::processDecompressedData(ostream *out, string str, const shared_ptr<IndexEntry> &startingIndexLine) {
    vector<string> splitLines = splitStr(str);
    totalSplitCount += splitLines.size();

    string curIncompleteLastLine;
    // Strip away incomplete line, store this line for the next block.
    string lastSplitLine;
    if (!splitLines.empty())
        lastSplitLine = splitLines[splitLines.size() - 1];
    char lastChar{0};
    if (!str.empty())
        lastChar = str[str.size() - 1];
    if (lastChar != '\n') {
        curIncompleteLastLine = lastSplitLine;
        if (!splitLines.empty())
            splitLines.pop_back();
        totalSplitCount--;
    }

    if (splitLines.empty()) {
        incompleteLastLine = incompleteLastLine + curIncompleteLastLine;
        return false;
    }

    bool result = true;
    if (skip < splitLines.size()) {

        // Two options. Extraction began earlier OR we are processing another chunk of data.
        u_int64_t iStart = 0;
        if (extractedLines == 0) {
            if (startingIndexLine->offsetOfFirstValidLine > 0) {
                if (firstPass) {
                    if (!splitLines.empty()) splitLines.erase(splitLines.begin());
                    iStart = skip;
                } else if (!incompleteLastLine.empty() && extractedLines < lineCount) {
                    storeOrOutputLine(out, &skipLines, incompleteLastLine + splitLines[0]);
                    extractedLines++;
                    iStart = 1;
                }
            } else
                iStart = skip;
        } else {
            if (!incompleteLastLine.empty() && extractedLines < lineCount) {
                storeOrOutputLine(out, &skipLines, incompleteLastLine + splitLines[0]);
                extractedLines++;
                iStart = 1;
            }
        }
        // iStart is 0 or 1
        for (int i = iStart; i < splitLines.size() && extractedLines < lineCount; ++i) {
            storeOrOutputLine(out, &skipLines, splitLines[i]);
            extractedLines++;
        }
    } else {
        result = false;
    }
    incompleteLastLine = curIncompleteLastLine;
    if (skip > 0) skip -= min(splitLines.size(), skip);

    return result;
}

void Extractor::storeOrOutputLine(ostream *outStream, uint64_t *skipLines, string line) {
    if (*skipLines > 0) {
        (*skipLines)--;
//        debug(string("skip line: ") + line);
    } else {
        if (enableDebugging)
            storedLines.emplace_back(line);
        else
            (*outStream) << line << "\n";
    }
}

void Extractor::storeLinesOfCurrentBlockForDebugMode() {
    if (!enableDebugging) return;

    while (storedLines.size() > lineCount) {
        storedLines.pop_back();
    }
}

vector<string> Extractor::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexReader->getErrorMessages();
    return mergeToNewVector(l, r);
}
