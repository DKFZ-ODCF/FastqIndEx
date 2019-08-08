/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXENTRYSTORAGESTRATEGY_H
#define FASTQINDEX_INDEXENTRYSTORAGESTRATEGY_H

#include "common/CommonStructsAndConstants.h"
#include "common/IndexEntryV1.h"
#include <regex>
#include <string>

using namespace std;

/**
 * Base class for various storage decision strategies for index entries.
 */
class IndexEntryStorageStrategy {

protected:

    shared_ptr<IndexEntryV1> lastStoredEntry;

    /**
     * Empty blocks occur a lot and will cause trouble, if they get written to an index file. To overcome this, we skip
     * empty blocks until the next valid block.
     */
    bool postponeWrite{false};

public:

    /**
     * Call this to see, if the current block can be used as a reference for an index entry.
     *
     * This method WILL have side effects, as e.g. the index entry and the postponeWrite flag will be stored / modified.
     * @param indexEntry The index entry object for which the decision is done
     * @param position   The position in the compressed source file
     * @param block      The compressed blocks number
     * @return
     */
    virtual bool shallStore(shared_ptr<IndexEntryV1> indexEntry, int64_t blockID, bool blockIsEmpty) = 0;

    /**
     * Set the internal postponeWrite variable to false;
     */
    void resetPostponeWrite() {
        this->postponeWrite = false;
    }

    /**
     * If implemented by the sub class, this might lead to a modification of the behaviour of shallStore. So call this
     * BEFORE you call shallStore.
     * @param filesize  The size of the file for which this strategy is used.
     */
    virtual void useFileSizeForCalculation(int64_t filesize) {};

    bool wasPostponed() { return postponeWrite; }
};

/**
 * Strategy to store and entry for every n'th (when applicable) compressed block applying (if set) a safe distance
 * check. Empty blocks will not be used as a reference point for an index entry.
 */
class BlockDistanceStorageStrategy : public IndexEntryStorageStrategy {

protected:

    int blockInterval{0};

    /**
     * Set to true, to enable an extended check, if the Byte-distance
     */
    bool useSafeDistanceCheck{false};

public:

    static const uint DEFAULT_BLOCKINTERVAL = 2048;

    static shared_ptr<BlockDistanceStorageStrategy> getDefault() {
        return from(-1);
    }

    static shared_ptr<BlockDistanceStorageStrategy> from(int blockInterval, bool useSafeDistanceCheck = true) {
        return make_shared<BlockDistanceStorageStrategy>(blockInterval, useSafeDistanceCheck);
    }

    static u_int32_t calculateIndexBlockInterval(int64_t fileSize) {
        int64_t testSize = fileSize / GB;

        if (testSize <= 1)
            return 16;
        if (testSize <= 2)
            return 32;
        if (testSize <= 4)
            return 64;
        if (testSize <= 8)
            return 128;
        if (testSize <= 16)
            return 256;
        if (testSize <= 32)
            return 512;
        if (testSize <= 64)
            return 1024;
        if (testSize <= 128)
            return DEFAULT_BLOCKINTERVAL;
        if (testSize <= 256)
            return 4096;
        return 8192;
    }

    BlockDistanceStorageStrategy(int blockInterval, bool useSafeDistanceCheck) {
        this->blockInterval = blockInterval;
        this->useSafeDistanceCheck = useSafeDistanceCheck;
        if (this->blockInterval <= 0)
            this->blockInterval = -1;
    }

    int getBlockInterval() {
        return blockInterval;
    }

    /**
     * It is allowed to have the blockInterval set to -1. In that case it is allowed to automatically
     * set the value by calling useFilesizeForCalculation(). However, if it is still -1, we apply a default value here.
     */
    bool shallStore(shared_ptr<IndexEntryV1> indexEntry, int64_t blockID, bool blockIsEmpty) override {
        if (blockInterval == -1) {
            blockInterval = DEFAULT_BLOCKINTERVAL;
        }

        // Only write back every nth entry. As we need the large window / dictionary of 32kb, we'll need to save
        // some space here. Think of large NovaSeq FASTQ files with ~200GB! We can't store approximately 3.6m index
        // entries which would be around 115GB for an index file

        // In addition to the block distance, we need to add a further constrant.
        // E.g. in test files from here:
        // https://trace.ncbi.nlm.nih.gov/Traces/sra/sra.cgi?exp=SRX000001&cmd=search&m=downloads&s=seq
        //  What I found out is, that the checked files contain very small compressed blocks each holding just a few bytes.
        //  To encounter this, we introduce the failsafe distance when writing index entries. So not only the block distance
        //  will be looked at but also the Byte distance to the last written index entry.
        // Why?  In one exemplary case, the file size is approximately 145MB and it contains around 7,35 million compressed
        //  blocks. This results in ca. 20Bytes per block. E.g. storing every 16th block with ~ 32kByte results in a
        //  hopelessly huge index file.

        auto blockOffset = indexEntry->blockOffsetInRawFile;
        u_int64_t offsetOfLastEntry = 0;
        bool failsafeDistanceIsReached = true;
        if (useSafeDistanceCheck) {
            u_int64_t failsafeDistance = static_cast<u_int64_t>(blockInterval) * 16384;
            if (lastStoredEntry != nullptr)
                offsetOfLastEntry = lastStoredEntry->blockOffsetInRawFile;
            failsafeDistanceIsReached = (blockOffset - offsetOfLastEntry) > (failsafeDistance);
        }

        bool shouldWrite = postponeWrite || blockID == 0 || (blockID % blockInterval == 0 && failsafeDistanceIsReached);
        if (blockIsEmpty && shouldWrite) {
            postponeWrite = true;
            shouldWrite = false;
        }

        if (shouldWrite) {
            this->lastStoredEntry = indexEntry;
        }
        return shouldWrite;
    }

    void useFileSizeForCalculation(int64_t filesize) override {
        if (blockInterval == -1) {
            if (filesize == 0) {
                blockInterval = 2048; // Like for files > 128GB.
            } else {
                blockInterval = calculateIndexBlockInterval(filesize);
            }
        }
    }
};

/**
 * Storage decision strategy, which looks mainly at the current distance to the last index entry reference point
 * (position in the compressed source file)
 */
class ByteDistanceStorageStrategy : public IndexEntryStorageStrategy {

private:
    /**
     * Minimum! distance between two compressed source blocks referenced by an index entry in Byte.
     */
    int64_t minIndexEntryByteDistance{0};

public:

    static const int64_t DEFAULT_MININDEXENTRY_BYTEDISTANCE = 256 * 1024 * 1024; // Can't use extern const here.

    static shared_ptr<ByteDistanceStorageStrategy> getDefault() {
        return from("1G");
    }

    static shared_ptr<ByteDistanceStorageStrategy> from(const string &value) {
        return make_shared<ByteDistanceStorageStrategy>(value);
    }

    static int64_t parseStringValue(const string &str) {
        if (str == "-1")
            return -1;

        long long result = 0;
        if (regex_match(str.c_str(), regex("[0-9]+[kmgtKMGT]"))) {
            result = stoll(str.substr(0, str.length() - 1));
            char unit = static_cast<char>(tolower(str.c_str()[str.length() - 1]));
            if (unit == 'k') {
                result *= kB;
            } else if (unit == 'm') {
                result *= MB;
            } else if (unit == 'g') {
                result *= GB;
            } else if (unit == 't') {
                result *= TB;
            }
        } else if (std::regex_match(str.c_str(), regex("[0-9]+"))) {
            result = stoll(str) * MB;
        }

        if (result <= 0) return -1;

        return result;
    }

    static int64_t calculateDistanceBasedOnFileSize(int64_t fileSize) {
        int64_t distance = fileSize / 512;

        if (distance <= 256 * kB)
            return 256 * kB;

        return distance;
    }

    explicit ByteDistanceStorageStrategy(int64_t minIndexEntryByteDistance) {
        this->minIndexEntryByteDistance = minIndexEntryByteDistance;
        if (this->minIndexEntryByteDistance <= 0)
            this->minIndexEntryByteDistance = -1;
    }

    explicit ByteDistanceStorageStrategy(const string &minIndexEntryByteDistance) {
        this->minIndexEntryByteDistance = parseStringValue(minIndexEntryByteDistance);
        if (this->minIndexEntryByteDistance <= 0)
            this->minIndexEntryByteDistance = -1;
    }

    int64_t getMinIndexEntryByteDistance() {
        return minIndexEntryByteDistance;
    }

    /**
     * It is allowed to have the minIndexEntryByteDistance set to -1. In that case it is allowed to automatically
     * set the value by calling useFilesizeForCalculation(). However, if it is still -1, we apply a default value here.
     */
    bool shallStore(shared_ptr<IndexEntryV1> indexEntry, int64_t blockID, bool blockIsEmpty) override {
        if (minIndexEntryByteDistance == -1)
            minIndexEntryByteDistance = DEFAULT_MININDEXENTRY_BYTEDISTANCE; // Set to default! Shall we warn here?

        if (blockIsEmpty)
            return false;

        // Obviously the first entry, always store it.
        if (!this->lastStoredEntry) {
            this->lastStoredEntry = indexEntry;
            return true;
        }

        bool shallWrite = false;

        u_int64_t blockOffsetDelta = indexEntry->blockOffsetInRawFile - this->lastStoredEntry->blockOffsetInRawFile;

        if (blockOffsetDelta > static_cast<uint64_t>(minIndexEntryByteDistance))
            shallWrite = true;

        if (shallWrite)
            this->lastStoredEntry = indexEntry;
        return shallWrite;
    }

    void useFileSizeForCalculation(int64_t filesize) override {
        if (minIndexEntryByteDistance == -1) {
            if (filesize == 0) {
                minIndexEntryByteDistance = DEFAULT_MININDEXENTRY_BYTEDISTANCE; // Like for files > 128GB.
            } else {
                minIndexEntryByteDistance = calculateDistanceBasedOnFileSize(filesize);
            }
        }
    }
};

#endif //FASTQINDEX_INDEXENTRYSTORAGESTRATEGY_H
