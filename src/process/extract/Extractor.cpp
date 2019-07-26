/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Extractor.h"
#include "common/IOHelper.h"
#include "common/StringHelper.h"
#include "runners/IndexStatsRunner.h"
#include "process/base/ZLibBasedFASTQProcessorBaseClass.h"
#include "process/io/PathSource.h"
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

Extractor::Extractor(const shared_ptr<Source> &fastqfile,
                     const shared_ptr<Source> &indexFile,
                     const shared_ptr<Sink> &resultSink,
                     bool forceOverwrite,
                     ExtractMode mode, u_int64_t start, u_int64_t count, uint extractionMulitplier,
                     bool enableDebugging) :
        ZLibBasedFASTQProcessorBaseClass(fastqfile, indexFile, enableDebugging),
        mode(mode), start(start), count(count) {
    this->indexReader = make_shared<IndexReader>(indexFile);
    this->resultSink = resultSink;
    this->extractionMultiplier = extractionMulitplier == 0 ? 4 : extractionMulitplier;
    this->roundtripBuffer = new string[extractionMulitplier];

}

Extractor::~Extractor() { delete[] roundtripBuffer; }

bool Extractor::checkPremises() {
    if (mode == ExtractMode::lines && count == 0) {
        addErrorMessage("Can't extract a line count of 0 lines. The value needs to be a positive number.");
        return false;
    }

    if (mode == ExtractMode::segment && start >= count) {
        addErrorMessage("The specified segment exceeds the segment count.");
        return false;
    }

    if (!resultSink->checkPremises()) {
        return false;
    }


    bool couldOpen = this->indexReader->tryOpenAndReadHeader();

    if (!couldOpen)
        return false;

    // Check line counts and extraction multiplier!
    auto totalLines = indexReader->getIndexHeader().linesInIndexedFile;
    if (totalLines % extractionMultiplier != 0) {
        addErrorMessage(string("The total number of lines cannot be divided fully by the extraction multiplier.") +
                        "The total number of lines in the source files must be a multiple of the extraction multiplier.");
        return false;
    }
    return true;
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

void Extractor::calculateStartingLineAndLineCount() {
    if (mode == ExtractMode::lines) {
        startingLine = start;
        lineCount = count;
    } else if (mode == ExtractMode::segment) {
        auto totalLines = indexReader->getIndexHeader().linesInIndexedFile;
        auto totalRecords = totalLines / extractionMultiplier;
        auto recordsPerSegment = totalRecords / count;
        auto leftoverRecords = totalRecords % count;
        auto recordsInLastSegment = recordsPerSegment + leftoverRecords;

        lineCount = recordsPerSegment * extractionMultiplier;
        startingLine = start * lineCount;
        if (start == count - 1) { // Last segment
            lineCount = recordsInLastSegment * extractionMultiplier;
        }
    }
}

bool Extractor::extract() {
    auto resultFileStream = shared_ptr<ofstream>(nullptr);
    ofstream outfilestream;
    ostream *out;
    if (!useFile)
        out = &cout;
    else {
        outfilestream.open(resultSink->toString());
        out = &outfilestream;
    }

    calculateStartingLineAndLineCount();

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
    }

    if (!initializeZStreamForRawInflate()) {
        return false;
    }

    fastqFile->open();
    off_t initialOffset = startingIndexLine->blockOffsetInRawFile;
    totalBytesIn += initialOffset;
    int startBits = startingIndexLine->bits;
    if (startBits > 0)
        initialOffset--;
    fastqFile->setReadStart(initialOffset); // This is for S3. Could be integrated into seek. Dont' know yet.
    zlibResult = fastqFile->seek(initialOffset, true);
    if (zlibResult == -1) {
        addErrorMessage("");
        fastqFile->close();
        return false;
    }

    if (startBits > 0) {
        int ret = fastqFile->readChar();
        totalBytesIn++;
        if (ret == -1) {
            ret = fastqFile->lastError() ? Z_ERRNO : Z_DATA_ERROR;
            errorWasRaised = true;
        }
        // This will pop up a clang-tidy warning, but as Mark Adler does it, I don't want to change it.
        zlibResult = inflatePrime(&zStream, startBits, ret >> (8 - startBits));
    }
    if (startingIndexLine->compressedDictionarySize > 0) {
        // Decompress!
        Bytef uncompressedDictionary[WINDOW_SIZE]{0};
        uLongf destLen = WINDOW_SIZE;
        uLong sourceLen = startingIndexLine->compressedDictionarySize;
        auto result = uncompress2(uncompressedDictionary, &destLen, startingIndexLine->window, &sourceLen);
        zlibResult = inflateSetDictionary(&zStream, uncompressedDictionary, WINDOW_SIZE);
    } else {
        zlibResult = inflateSetDictionary(&zStream, startingIndexLine->window, WINDOW_SIZE);
    }

    if (errorWasRaised) {
        addErrorMessage(string("zlib reported an error: ") + zStream.msg);
        fastqFile->close();
        return false;
    }

    timerRestart("Final extractor init");

    IndexStatsRunner::printIndexEntryToConsole(startingIndexLine, indexEntryNumber, true);

    // The number of lines which will be skipped in the found starting block
    skip = startingLine - startingIndexLine->startingLineInEntry;

    bool keepExtracting = false;
    bool finalAbort = false;
    do {
        do {

            if (!readCompressedDataFromSource()) {
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

                if (!processDecompressedChunkOfData(out, currentDecompressedBlock.str(), startingIndexLine))
                    continue;

                // Tell the extractor, that the inner loop was called at least once, so we don't remove the first line of
                // the splitLines on every iteration.
                firstPass = false;

            } while (zStream.avail_in != 0 && zlibResult != Z_STREAM_END);
            finalAbort = !(extractedLines < lineCount && !errorWasRaised);
        } while (!finalAbort && zlibResult != Z_STREAM_END);

        storeLinesOfCurrentBlockForDebugMode();

        keepExtracting = checkAndPrepareForNextConcatenatedPart(finalAbort);
    } while (keepExtracting);

    // Free the file pointer and close the file.
    fastqFile->close();

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
    fastqFile->seek(streamEndPosition, true);

    if (!fastqFile->canRead()) return false;

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

bool
Extractor::processDecompressedChunkOfData(ostream *out, string str, const shared_ptr<IndexEntry> &startingIndexLine) {
    if (extractedLines >= lineCount)
        return false;
    vector<string> splitLines = StringHelper::splitStr(str);
    totalSplitCount += splitLines.size();

    // In the case, that we invoke this method the first time, the index entry
    bool removeIncompleteFirstLine = firstPass && startingIndexLine->offsetOfFirstValidLine > 0;
    if (removeIncompleteFirstLine) splitLines.erase(splitLines.begin());
    firstPass = false;

    // Strip away incomplete last line, store this line for the next block.
    string curIncompleteLastLine;

    char lastChar{0};
    if (!str.empty())
        lastChar = str[str.size() - 1];
    if (lastChar != '\n') {
        if (!splitLines.empty()) {
            curIncompleteLastLine = splitLines[splitLines.size() - 1];
            splitLines.pop_back();
        }
        totalSplitCount--;
    }

    // Even if the split string fails, there could still be a newline in the string. Add this and continue.
    if (splitLines.empty()) {
        incompleteLastLine += curIncompleteLastLine;
        return false;
    }

    bool result = true;
    // Basically two cases, first case, we have enough data here and can output something, or we skip the whole chunk
    if (skip >= splitLines.size()) {    // Ignore
        result = false;
    } else {                            // Output
        u_int64_t iStart = skip;
        if (iStart == 0) { // We start right away, also use incompleteLastLine.
            storeOrOutputLine(out, incompleteLastLine + splitLines[0]);
            extractedLines++;
            iStart = 1;
        }
        // iStart is 0 or 1
        for (int i = iStart; i < splitLines.size() && extractedLines < lineCount; ++i) {
            storeOrOutputLine(out, splitLines[i]);
            extractedLines++;
        }
    }
    incompleteLastLine = curIncompleteLastLine;
    if (skip > 0) skip -= min(splitLines.size(), skip);

    return result;
}

void Extractor::storeOrOutputLine(ostream *outStream, string line) {
    roundtripBufferPosition = extractedLines % extractionMultiplier;
    roundtripBuffer[roundtripBufferPosition] = line;

    if (enableDebugging)
        storedLines.emplace_back(line);
    else
        (*outStream) << line << "\n";
}

void Extractor::storeLinesOfCurrentBlockForDebugMode() {
    if (!enableDebugging) return;

    while (storedLines.size() > lineCount) {
        storedLines.pop_back();
    }
}

vector<string> Extractor::getErrorMessages() {
    vector<string> a = ErrorAccumulator::getErrorMessages();
    vector<string> b = indexReader->getErrorMessages();
    vector<string> c = resultSink->getErrorMessages();
    return mergeToNewVector(a, b, c);
}
