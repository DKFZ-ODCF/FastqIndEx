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
#include <experimental/filesystem>
#include <iostream>
#include <zlib.h>

using namespace experimental::filesystem;
using namespace std;
using namespace std::chrono;

using experimental::filesystem::path;

Extractor::Extractor(const shared_ptr<Source> &fastqfile,
                     const shared_ptr<Source> &indexFile,
                     const shared_ptr<Sink> &resultSink,
                     bool forceOverwrite,
                     ExtractMode mode, int64_t start, int64_t count, uint recordSize,
                     bool enableDebugging) :
        ZLibBasedFASTQProcessorBaseClass(fastqfile, indexFile, enableDebugging),
        mode(mode), start(start), count(count) {
    this->indexReader = make_shared<IndexReader>(indexFile);
    this->resultSink = resultSink;
    this->recordSize = recordSize == 0 ? DEFAULT_RECORD_SIZE : recordSize;
    this->roundtripBuffer = new string[recordSize];
    this->forceOverwrite = forceOverwrite;
}

Extractor::~Extractor() { delete[] roundtripBuffer; }

bool Extractor::fulfillsPremises() {
    if (mode == ExtractMode::lines && count == 0) {
        addErrorMessage("Can't extract a line count of 0 lines. The value needs to be a positive number.");
        return false;
    }

    if (mode == ExtractMode::segment && start >= count) {
        addErrorMessage("The specified segment number '", to_string(start + 1), "' exceeds the segment count of '",
                        to_string(count), "'.");
        return false;
    }

    if (!resultSink->fulfillsPremises()) {
        return false;
    }


    bool couldOpen = this->indexReader->tryOpenAndReadHeader();

    if (!couldOpen)
        return false;

    // Check line counts and record size!
    auto totalLines = indexReader->getIndexHeader().linesInIndexedFile;
    if (totalLines % recordSize != 0) {
        addErrorMessage("The total number of lines '", to_string(totalLines),
                        "'is not a multiple of the record size '", to_string(recordSize), "'.");
        return false;
    }
    return true;
}

void Extractor::calculateStartingLineAndLineCount() {
    if (mode == ExtractMode::lines) {
        startingLine = start;
        lineCount = count;
    } else if (mode == ExtractMode::segment) {
        auto totalLines = indexReader->getIndexHeader().linesInIndexedFile;
        auto totalRecords = totalLines / recordSize;
        auto recordsPerSegment = totalRecords / count;
        auto leftoverRecords = totalRecords % count;
        auto recordsInLastSegment = recordsPerSegment + leftoverRecords;

        lineCount = recordsPerSegment * recordSize;
        startingLine = start * lineCount;
        if (start == count - 1) { // Last segment
            lineCount = recordsInLastSegment * recordSize;
        }
    }
}

void Extractor::findIndexEntryForExtraction() {
    shared_ptr<IndexEntry> previousEntry = indexReader->readIndexEntry();
    shared_ptr<IndexEntry> latestIndexEntry = previousEntry;

    int64_t latestIndexEntryNumber = 0;
    while (indexReader->getIndicesLeft() > 0) {
        auto entry = indexReader->readIndexEntry();
        latestIndexEntryNumber++;
        if (entry->startingLineInEntry > startingLine) {
            break;
        }
        latestIndexEntry = entry;
    }
    this->usedIndexEntry = latestIndexEntry;
    this->usedIndexEntryNumber = latestIndexEntryNumber;
}

bool Extractor::openFastqAndPrepareZStream() {
    fastqFile->open();
    off_t initialOffset = usedIndexEntry->blockOffsetInRawFile;
    totalBytesIn += initialOffset;
    int startBits = usedIndexEntry->bits;
    if (startBits > 0)
        initialOffset--;
    fastqFile->setReadStart(initialOffset); // This is for S3. Could be integrated into seek. Dont' know yet.
    auto seekResult = fastqFile->seek(initialOffset, true);
    if (seekResult == -1) {
        addErrorMessage("Could not jump to position '", to_string(initialOffset),
                        "' in file '", fastqFile->toString(), "'");
        return false;
    }

    if (startBits > 0) {
        int ret = fastqFile->readChar();
        totalBytesIn++;
        if (ret == -1) {
            ret = fastqFile->lastError() ? Z_ERRNO : Z_DATA_ERROR;
            addErrorMessage("Could not read from FASTQ file '", fastqFile->toString(),
                    "'. The latest zlib error code was '", to_string(ret), "'.");
            return false;
        }
        // The following line will pop up a clang-tidy warning, but as Mark Adler does it, I don't want to change it.
        zlibResult = inflatePrime(&zStream, startBits, ret >> (8 - startBits));
        if(zlibResult != 0) {
            addErrorMessage("Could not prime the zStream for extraction. zlib reported: '", zStream.msg, "'.");
            return false;
        }
    }
    return true;
}

bool Extractor::setDictionaryForZStream() {
    if (usedIndexEntry->compressedDictionarySize > 0) {
        // Decompress!
        Bytef uncompressedDictionary[WINDOW_SIZE]{0};
        uLongf destLen = WINDOW_SIZE;
        uLong sourceLen = usedIndexEntry->compressedDictionarySize;
        // Ignore result here as zlib will fail anyways upon inflateSetDictionary, if the step went wrong.
        uncompress2(uncompressedDictionary, &destLen, usedIndexEntry->window, &sourceLen);
        zlibResult = inflateSetDictionary(&zStream, uncompressedDictionary, WINDOW_SIZE);
    } else {
        zlibResult = inflateSetDictionary(&zStream, usedIndexEntry->window, WINDOW_SIZE);
    }
    if(zlibResult != 0) {
        addErrorMessage("There was an error when trying to set to dictionary for decompression. zlib reported: '", zStream.msg, "'.");
        return false;
    }
    return true;
}

bool Extractor::extract() {
    if (!initializeZStreamForRawInflate())
        return false;

    calculateStartingLineAndLineCount();

    findIndexEntryForExtraction();

    if(!openFastqAndPrepareZStream() ||
       !setDictionaryForZStream()) {
        fastqFile->close();
        return false;
    }

    IndexStatsRunner::printIndexEntryToConsole(usedIndexEntry, usedIndexEntryNumber, true);

    // The number of lines which will be skipped from the beginning of the referenced compressed block.
    skip = startingLine - usedIndexEntry->startingLineInEntry;

    bool keepExtracting{false};
    bool finalAbort{false};
    do {
        do {
            bool readCompressedDataWasSuccessful = readCompressedDataFromSource();
            if (!readCompressedDataWasSuccessful) {
                errorWasRaised = true;
                break;
            }

            // Process read data or until end of stream
            do {
                resetSlidingWindowIfNecessary();

                clearCurrentCompressedBlock();

                bool checkForStreamEnd = false;
                if (!decompressNextChunkOfData(checkForStreamEnd, Z_NO_FLUSH))
                    break;

                if (!processDecompressedChunkOfData(currentDecompressedBlock.str(), usedIndexEntry))
                    continue;

                // Tell the extractor, that the inner loop was called at least once, so we don't remove the first line of
                // the splitLines on every iteration.
                firstPass = false;

            } while (zStream.avail_in != 0 && zlibResult != Z_STREAM_END);
            finalAbort = !(extractedLines < lineCount && !errorWasRaised);
        } while (!finalAbort && zlibResult != Z_STREAM_END);

        storeLinesOfCurrentBlockForDebugMode();

        keepExtracting = prepareForNextConcatenatedPartIfNecessary(finalAbort);
    } while (keepExtracting);

    // Free the file pointer and close the file.
    fastqFile->close();

    resultSink->close();

    inflateEnd(&zStream);

    if (errorWasRaised)
        addErrorMessage(string("Last error message from zlib: ") + zStream.msg);

    return !errorWasRaised;
}

bool Extractor::prepareForNextConcatenatedPartIfNecessary(bool finalAbort) {
    if (finalAbort) return false;

    totalBytesIn += 8 + 10; // Plus 8 Byte (for what? they are missing...) and 10 Byte for the next header
    int64_t streamEndPosition = totalBytesIn;
    fastqFile->seek(streamEndPosition, true);

    if (!fastqFile->canRead()) return false;

    inflateEnd(&zStream);
    if (!initializeZStreamForRawInflate()) {
        finishedSuccessful = false;
        return false;
    }

    resetSlidingWindowIfNecessary();
    memset(window, 0, WINDOW_SIZE);
    memset(input, 0, CHUNK_SIZE);
    zStream.avail_in = CHUNK_SIZE;
    firstPass = true;

    Bytef dict[WINDOW_SIZE]{0};
    zlibResult = inflateSetDictionary(&zStream, dict, WINDOW_SIZE);

    return true;
}

bool Extractor::processDecompressedChunkOfData(const string &str, const shared_ptr<IndexEntry> &startingIndexLine) {
    if (extractedLines >= lineCount)
        return false;
    vector<string> splitLines = StringHelper::splitStr(str);
    totalSplitCount += splitLines.size();

    // In the case, that we invoke this method the first time, the index entry
    bool removeIncompleteFirstLine = firstPass && startingIndexLine->offsetToNextLineStart > 0;
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
        int64_t iStart = skip;
        if (iStart == 0) { // We start right away, also use incompleteLastLine.
            storeOrOutputLine(incompleteLastLine + splitLines[0]);
            extractedLines++;
            iStart = 1;
        }
        // iStart is 0 or 1
        for (u_int64_t i = iStart; i < splitLines.size() && extractedLines < lineCount; ++i) {
            storeOrOutputLine(splitLines[i]);
            extractedLines++;
        }
    }
    incompleteLastLine = curIncompleteLastLine;
    if (skip > 0) skip -= min(splitLines.size(), skip);

    return result;
}

void Extractor::storeOrOutputLine(const string &line) {
    // For later! Roundtrip buffers could be an easy option to recognize if an empty line was forgotten.
    //    roundtripBufferPosition = extractedLines % recordSize;
    //    roundtripBuffer[roundtripBufferPosition] = line;

    if (enableDebugging)
        storedLines.emplace_back(line);
    else {
        resultSink->write(line);
        resultSink->write("\n");
    }
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
