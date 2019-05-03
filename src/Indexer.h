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
#include "ZLibBasedFASTQProcessorBaseClass.h"
#include <zlib.h>
#include <string>

using namespace std;

/**
 * The Indexer class is used to walk through a gz compressed FASTQ file and to write an index for this file.
 * An Indexer is a one-time-use only object! Attempts to reuse it will fail.
 * While tryOpen() and checkPremises() will call thread / interprocess safe check and access methods for the index file,
 * all other methods are not. However, createIndex() will call tryOpen() and fail, if a write lock is already active.
 * It is also not allowed to rerun createIndex().
 */
class Indexer : public ZLibBasedFASTQProcessorBaseClass {
public:

    /**
     * This is the version of the current Indexer implementation. In contrary to the Extractor, there is always only one
     * Indexer version available.
     * If the Indexer ever changes, keep in mind to increment this version! This will be used to select the appropriate
     * Extractor class when an index is read!
     */
    static const unsigned int INDEXER_VERSION;

    static uint calculateIndexBlockInterval(u_int64_t fileSize);

private:

    long numberOfFoundEntries = 0;

    shared_ptr<IndexWriter> indexWriter;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * keeps the index header
     */
    shared_ptr<IndexHeader> storedHeader = shared_ptr<IndexHeader>(nullptr);

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * Keeps all generated index entries
     */
    vector<shared_ptr<IndexEntryV1>> storedEntries;

    /**
     * Current bits for the next index entry.
     */
    int curBits{0};

    /**
     * Current offset in file.
     */
    long offset{0};

    /**
     * Marks, if the last inflated block ended with the newline character.
     * If true, the blockOffsetInRawFile for the first line in the new block will be 0.
     * If false, we have to look for the byte blockOffsetInRawFile.
     */
    bool lastBlockEndedWithNewline = true;

    u_int64_t lineCountForNextIndexEntry{0};

    long blockID{-1};                   // Number of the currently processed block.

    int blockInterval;                  // Only store every n'th index entry.

    shared_ptr<IndexEntryV1> lastStoredEntry;

    /**
     * When we write out a new index entry, we need a dictionary. This is taken from the block of data before the block
     * to which the new entry references.
     */
    Bytef dictionaryForNextBlock[WINDOW_SIZE]{0};

public:


    /**
     * Be careful, when you enableDebugging. This will tell the Indexer to store information about the process, which can
     * e.g. be used for unit test. E.g. this will store ALL lines found in the FASTQ file! So it is absolutely not
     * advisable to use it with large FASTQ files. For test data it is safe to use.
     * @param fastq The FASTQ we are working on.
     * @param index The index for the FASTQ.
     * @param enableDebugging Store debug information or not.
     */
    Indexer(const shared_ptr<InputSource> &fastqfile, const path &index, int blockInterval,
            bool enableDebugging = false, bool forceOverwrite = false);

    virtual ~Indexer() = default;

    bool checkPremises();

    /**
     * This will create an IndexHeader instance with INDEXER_VERSION and the size of the used IndexEntry struct.
     */
    shared_ptr<IndexHeader> createHeader();

    /**
     * Start the index creation,
     * @return true, if everything went fine.
     */
    bool createIndex();

    void finalizeProcessingForCurrentBlock(stringstream &currentDecompressedBlock, z_stream *strm);

    void storeLinesOfCurrentBlockForDebugMode(std::stringstream &currentDecompressedBlock);

    /**
     * Overriden to also pass through (copywise, safe but slow but also only with a few entries and in error cases)
     * error messages from the used IndexWriter and ZLibHelper instance.
     * @return Merged vector of the objects error messages + the index writers error messages.
     */
    vector<string> getErrorMessages() override;

    long getFoundEntries() { return numberOfFoundEntries; };

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * @return The IndexHeader, which was created during createIndex()
     */
    shared_ptr<IndexHeader> getStoredHeader() { return storedHeader; };

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * @return The index entries, which were created during the index run.
     */
    const vector<shared_ptr<IndexEntryV1>> &getStoredEntries() { return storedEntries; }


};


#endif //FASTQINDEX_INDEXER_H
