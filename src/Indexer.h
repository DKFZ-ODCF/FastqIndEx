/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXER_H
#define FASTQINDEX_INDEXER_H

#include "CommonStructsAndConstants.h"
#include "ErrorAccumulator.h"
#include "IndexWriter.h"
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <zlib.h>
#include <string>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::ptr_container;

/**
 * The Indexer class is used to walk through a gz compressed FASTQ file and to write an index for this file.
 * An Indexer is a one-time-use only object! Attempts to reuse it will fail.
 * While tryOpen() and checkPremises() will call thread / interprocess safe check and access methods for the index file,
 * all other methods are not. However, createIndex() will call tryOpen() and fail, if a write lock is already active.
 * It is also not allowed to rerun createIndex().
 */
class Indexer : public ErrorAccumulator {
public:

    /**
     * This is the version of the current Indexer implementation. In contrary to the Extractor, there is always only one
     * Indexer version available.
     * If the Indexer ever changes, keep in mind to increment this version! This will be used to select the appropriate
     * Extractor class when an index is read!
     */
    static const unsigned int INDEXER_VERSION;

    static uint calculateIndexBlockInterval(ulong fileSize);

private:

    path fastq;

    path index;

    bool finishedSuccessful = false;

    long numberOfFoundEntries = 0;

    bool debuggingEnabled = false;

    /**
     * Set to true, as soon as createIndex() was started.
     */
    bool wasStarted = false;

    boost::shared_ptr<IndexWriter> indexWriter;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * keeps the index header
     */
    boost::shared_ptr<IndexHeader> storedHeader = boost::shared_ptr<IndexHeader>(nullptr);

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * Keeps all generated index entries
     */
    vector<boost::shared_ptr<IndexEntryV1>> storedEntries;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * Keeps all lines read from the fastq file.
     */
    vector<string> storedLines;


    // Variables used in createIndex() and subsequent methods.
    uint totalBytesIn{0};        /* our own total counters to avoid 4GB limit, as taken from zran example */

    uint totalBytesOut{0};

    uint lastBytesIn{0};

    int curBits{0};

    long offset{0};

    // Marks, if the last inflated block ended with the newline character.
    // If true, the blockOffsetInRawFile for the first line in the new block will be 0.
    // If false, we have to look for the byte blockOffsetInRawFile.
    bool lastBlockEndedWithNewline = true;

    bool firstBlock = true;             // Ignore the first block! Does not contain any data.

    ulong totalLineCount{0};            // The compression block we are in.

    long blockID{-1};                   // Number of the currently processed block.

    int blockInterval;                  // Only store every n'th index entry.

public:

    /**
     * Be careful, when you enableDebugging. This will tell the Indexer to store information about the process, which can
     * e.g. be used for unit test. E.g. this will store ALL lines found in the FASTQ file! So it is absolutely not
     * advisable to use it with large FASTQ files. For test data it is safe to use.
     * @param fastq The FASTQ we are working on.
     * @param index The index for the FASTQ.
     * @param enableDebugging Store debug information or not.
     */
    Indexer(const path &fastq, const path &index, int blockInterval, bool enableDebugging = false);

    virtual ~Indexer();

    bool checkPremises();

    /**
     * Overriden to also pass through (copywise, safe but slow but also only with a few entries and in error cases)
     * error messages from the used IndexWriter instance.
     * @return Merged vector of the objects error messages + the index writers error messages.
     */
    vector<string> getErrorMessages() override;

    bool isDebuggingEnabled() { return debuggingEnabled; }

    path getFastq() { return fastq; }

    path getIndex() { return index; }

    /**
     * This will create an IndexHeader instance with INDEXER_VERSION and the size of the used IndexEntry struct.
     */
    boost::shared_ptr<IndexHeader> createHeader();

    /**
     * Will create a struct instance of type z_stream, initialise some fields like to be seen in
     * https://github.com/madler/zlib/blob/master/examples/zran.c
     *
     * @return true, if the strm initialize was successful, otherwise false.
     */
    bool initializeZStream(z_stream *strm);

    /**
     * Read a chunk of data from the z_strm strm and check for errors. If errors pop up, they are stored.
     * @param strm to read to
     * @return true, if everything went fine, otherwise false.
     */
    bool readCompressedDataFromStream(FILE *const inputFile, z_stream *strm, Byte *const buffer);

    /**
     * Start the index creation,
     * @return true, if everything went fine.
     */
    bool createIndex();

    bool checkStreamForBlockEnd(z_stream *strm) const;

    void finalizeProcessingForCurrentBlock(stringstream &currentDecompressedBlock, z_stream *strm);

    void storeLinesOfCurrentBlockForDebugMode(std::stringstream &currentDecompressedBlock);


    bool wasSuccessful() { return finishedSuccessful; };

    long getFoundEntries() { return numberOfFoundEntries; };

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * @return The IndexHeader, which was created during createIndex()
     */
    boost::shared_ptr<IndexHeader> getStoredHeader() { return storedHeader; };

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * @return The index entries, which were created during the index run.
     */
    const vector<boost::shared_ptr<IndexEntryV1>> &getStoredEntries() { return storedEntries; }

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * Be sure what you do, before you turn on enableDebugging! This will return ALL lines found in the FASTQ file!
     */
    const vector<string> &getStoredLines() { return storedLines; }
};


#endif //FASTQINDEX_INDEXER_H
