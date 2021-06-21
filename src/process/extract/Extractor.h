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
#include "process/io/FileSource.h"
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

    /**
     * If the extractor shall write a new fastq file, this indicates, that the file will be overwritten.
     */
    bool forceOverwrite{};

    /**
     * The index entry, which is closest to the requested starting line.
     * This value is set by findIndexEntryForExtraction()
     */
    shared_ptr<IndexEntry> usedIndexEntry;

    /**
     * A custom count for the index startingIndexEntry. It is for documentation.
     * This value is set by findIndexEntryForExtraction()
     */
    int64_t usedIndexEntryNumber{0};

    /**
     * If a block starts with an offset, this can be used to complete the unfinished line of the last block (if necessary)
     */
    string unfinishedLineInLastBlock;

    string incompleteLastLine;

    u_int64_t extractedLines = 0;

    /**
     * The number of lines which will be skipped in the found starting block
     */
    u_int64_t skip = 0;

    /**
     * Keep track of all split lines. Merely for debugging
     */
    u_int64_t totalSplitCount = 0;

    /**
     * The roundtrip buffer is a concept which might be implemented in the future. The idea behind this is, that the
     * roundtripBuffer is an array of size recordSize and if the extractor somehow misses to extract an empty line at
     * the end (Some FASTQs have a lot of empty lines), the extractor will still output a full entry.
     */
    string *roundtripBuffer = nullptr;

    /**
     * Current position in the roundtripBuffer between [0;recordSize[
     */
    uint roundtripBufferPosition = 0;

public:

    /**
     * @param sourceFile         The file from which we will extract data
     * @param indexFile         The index file for this file
     * @param resultSink        The result file or
     * @param forceOverwrite    If the resultfile exists, we can overwrite it with this flag
     * @param mode              Extraction mode, currently either lines or segment
     * @param start             Start extraction from this line or extract this segment
     * @param count             Extract a maximum of lineCount lines OR define the number of total segments
     * @param enableDebugging   Used for interactive debugging and unit tests
     */
    explicit Extractor(const shared_ptr<Source> &sourceFile,
                       const shared_ptr<Source> &indexFile,
                       const shared_ptr<Sink> &resultSink,
                       bool forceOverwrite,
                       ExtractMode mode, int64_t start, int64_t count, uint recordSize,
                       bool enableDebugging);

    ~Extractor() override;

    ExtractMode getExtractMode() { return mode; }

    shared_ptr<Sink> getResultSink() { return resultSink; }

    int64_t getStart() { return start; }

    int64_t getCount() { return count; }

    int64_t getStartingLine() { return startingLine; }

    int64_t getLineCount() { return lineCount; }

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
     * Find the index entry in the index file, which is closest to the starting line. Fills the variables:
     * - usedIndexEntry
     * - usedIndexEntryNumber
     */
    void findIndexEntryForExtraction();

    /**
     * Open the FASTQ file and prepare initially prepare the zStream.
     * If the method fails, sourceFile will not be closed automatically.
     * @return true, if the operation was successful.
     */
    bool openFastqAndPrepareZStream();

    /**
     * If the method fails, sourceFile will not be closed automatically.
     * @return true, if the operation was successful.
     */
    bool setDictionaryForZStream();

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
    bool processDecompressedChunkOfData(const string &str, const shared_ptr<IndexEntry> &startingIndexLine);

    bool prepareForNextConcatenatedPartIfNecessary(bool finalAbort);

    void storeOrOutputLine(const string &line);

    void storeLinesOfCurrentBlockForDebugMode();

    /**
     * Overridden to also pass through (each element is copied, this is safe and slower than references, but it is only
     * done with a few entries and in error cases) error messages from the used IndexReader and ZLibHelper instance.
     * @return Merged vector of the objects error messages + the index writers error messages.
     */
    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_EXTRACTORV1_H
