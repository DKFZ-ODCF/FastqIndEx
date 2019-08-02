/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXER_H
#define FASTQINDEX_INDEXER_H

#include "common/CommonStructsAndConstants.h"
#include "common/ErrorAccumulator.h"
#include "process/index/IndexEntryStorageStrategy.h"
#include "process/index/IndexWriter.h"
#include "process/io/Sink.h"
#include "process/base/ZLibBasedFASTQProcessorBaseClass.h"
#include <string>
#include <zlib.h>

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

private:

    long numberOfFoundEntries = 0;

    shared_ptr<IndexWriter> indexWriter;

    bool forbidWriteFQI;

    bool forceOverwrite{false};

    shared_ptr<IndexEntryStorageStrategy> storageStrategy;

    bool compressDictionaries{true};

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
     * For debug and test purposes, used when decompressed blocks are processed. Will store them
     * to a file next to the
     */
    bool writeOutOfDecompressedBlocksAndStatistics{false};

    path storageForDecompressedBlocks;

    bool writeOutOfPartialDecompressedBlocks{false};

    path storageForPartialDecompressedBlocks;

    ofstream partialBlockinfoStream;

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

    u_int64_t numberOfConcatenatedFiles{1};

    long blockID{-1};                   // Number of the currently processed block.

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
    Indexer(const shared_ptr<Source> &fastqfile,
            const shared_ptr<Sink> &index,
            shared_ptr<IndexEntryStorageStrategy> storageStrategy,
            bool enableDebugging = false,
            bool forceOverwrite = false,
            bool forbidWriteFQI = false,
            bool compressDictionaries = true
    );

    virtual ~Indexer() = default;

    void setDictionaryCompression(bool value) {
        this->compressDictionaries = value;
    }

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

    bool checkAndPrepareForNextConcatenatedPart();

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

    const uint64_t getNumberOfConcatenatedFiles() { return numberOfConcatenatedFiles; }

    shared_ptr<IndexEntryV1> createIndexEntryFromBlockData(const string &currentBlockString,
                                                           const vector<string> &lines,
                                                           u_int64_t &blockOffsetInRawFile,
                                                           bool lastBlockEndedWithNewline,
                                                           bool *currentBlockEndedWithNewLine,
                                                           u_int32_t *numberOfLinesInBlock);

    void storeDictionaryForEntry(z_stream *strm, shared_ptr<IndexEntryV1> entry);

    bool writeIndexEntryIfPossible(shared_ptr<IndexEntryV1> &entry, const vector<string> &lines, bool blockIsEmpty);

    void enableWriteOutOfDecompressedBlocksAndStatistics(const path &location) {
        this->writeOutOfDecompressedBlocksAndStatistics = true;
        this->storageForDecompressedBlocks = location;
    }


    void enableWriteOutOfPartialDecompressedBlocks(const path &location) {
        this->writeOutOfPartialDecompressedBlocks = true;
        this->storageForPartialDecompressedBlocks = location.u8string() + string("/blockinfo.txt");
    }
};


#endif //FASTQINDEX_INDEXER_H
