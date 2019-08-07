/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTORV1_H
#define FASTQINDEX_EXTRACTORV1_H

#include "common/CommonStructsAndConstants.h"
#include "common/ErrorAccumulator.h"
#include "process/extract/IndexReader.h"
#include "process/base/ZLibBasedFASTQProcessorBaseClass.h"
#include "process/io/PathSource.h"
#include <experimental/filesystem>
#include <zlib.h>

using namespace std;
using experimental::filesystem::path;

/**
 * Extraction can be either done on a per line / record base (starting record, number of records) or segement wise
 * (selected segment, number of segments).
 */
enum ExtractMode {
    lines,
    segment
};

class Extractor : public ZLibBasedFASTQProcessorBaseClass {

private:

    /**
     * Reader instance which will be used to read in the index.
     */
    shared_ptr<IndexReader> indexReader = shared_ptr<IndexReader>(nullptr);

    ExtractMode mode;

    /**
     * Either the segment to extract OR the starting line. This will be used to set startingLine upon extract.
     */
    u_int64_t start;

    /**
     * Either the number of segments OR the line count. This will be used to set startingLine AND lineCount upon extract.
     */
    u_int64_t count;

    /**
     * The first line which shall be extracted from the FASTQ. This is a multiple of recordSize.
     */
    u_int64_t startingLine{0};

    /**
     * The number of lines which shall be extracted from the FASTQ file. This is a multiple of recordSize.
     */
    u_int64_t lineCount{0};

    /**
     * The size of a record as in number of lines. E.g. a FASTQ record consists of four lines (which is also the default
     * in this application.
     */
    uint recordSize;

    /**
     * Defines, where the extracted data will be stored.
     */
    shared_ptr<Sink> resultSink;

//    bool useFile{false};

    /**
     * If the extractor shall write a new fastq file, this indicates, that the file will be overwritten.
     */
    bool forceOverwrite;

    // If a block starts with an offset, this can be used to complete the unfinished line of the last block (if necessary)
    string unfinishedLineInLastBlock;

    string incompleteLastLine;

    u_int64_t extractedLines = 0;

    // The number of lines which will be skipped in the found starting block
    u_int64_t skip = 0;

    // Keep track of all split lines. Merely for debugging
    u_int64_t totalSplitCount = 0;

    string *roundtripBuffer = nullptr;

    uint roundtripBufferPosition = 0;

public:

    /**
     * @param fastqFile         The file from which we will extract data
     * @param indexFile         The index file for this file
     * @param resultSink        The result file or
     * @param forceOverwrite    If the resultfile exists, we can overwrite it with this flag
     * @param mode              Extraction mode, currently either lines or segment
     * @param start             Start extraction from this line or extract this segment
     * @param count             Extract a maximum of lineCount lines OR define the number of total segments
     * @param enableDebugging   Used for interactive debugging and unit tests
     */
    explicit Extractor(const shared_ptr<Source> &fastqFile,
                       const shared_ptr<Source> &indexFile,
                       const shared_ptr<Sink> &resultSink,
                       bool forceOverwrite,
                       ExtractMode mode, u_int64_t start, u_int64_t count, uint recordSize,
                       bool enableDebugging);

    virtual ~Extractor();

    ExtractMode getExtractMode() { return mode; }

    shared_ptr<Sink> getResultSink() { return resultSink; }

    u_int64_t getStart() { return start; }

    u_int64_t getCount() { return count; }

    u_int64_t getStartingLine() { return startingLine; }

    u_int64_t getLineCount() { return lineCount; }

    /**
     * Will call tryOpenAndReadHeader on the internal indexReader.
     */
    bool fulfillsPremises();

    /**
     * For debugging and testing , will be overriden by extract(). Sets the initial amount of lines which will be
     * omitted.
     * TODO Move to test-aware subclass.
     */
    void setSkip(uint skip) {
        this->skip = skip;
    }

    /**
     * For debugging and testing , will be overriden by extract(). Sets the initial amount of lines which will be
     * omitted.
     * TODO Move to test-aware subclass.
     */
    void setFirstPass(bool firstPass) {
        this->firstPass = firstPass;
    }

    /**
     * Calculates the starting line and the line count based on the start, count and mode settings.
     */
    void calculateStartingLineAndLineCount();

    /**
     * Find the index entry in the index file, which is closest to the starting line.
     */
    tuple<u_int64_t, shared_ptr<IndexEntry>> findIndexEntryForExtraction();

    /**
     * For now directly to cout?
     * @param start
     * @param count
     */
    bool extract();

    /**
     * Process a decompressed chunk (NOT a complete decompressed block!) of data.
     *
     * Will reset the firstPass variable, if called for the first time.
     *
     * @param out The stream to put the data to
     * @param str The string of decompressed chunk data
     * @param startingIndexLine The index entry which was used to step into the gzip file.
     * @return true, if something was written out or false otherwise.
     */
    bool processDecompressedChunkOfData(string str, const shared_ptr<IndexEntry> &startingIndexLine);

    bool prepareForNextConcatenatedPartIfNecessary(bool finalAbort);

    void storeOrOutputLine(string line);

    void storeLinesOfCurrentBlockForDebugMode();

    /**
     * Overridden to also pass through (each element is copied, this is safe and slower than references, but it is only
     * done with a few entries and in error cases) error messages from the used IndexReader and ZLibHelper instance.
     * @return Merged vector of the objects error messages + the index writers error messages.
     */
    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_EXTRACTORV1_H
