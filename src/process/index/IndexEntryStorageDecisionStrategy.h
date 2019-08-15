/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXENTRYSTORAGEDECISIONSTRATEGY_H
#define FASTQINDEX_INDEXENTRYSTORAGEDECISIONSTRATEGY_H

#include "common/CommonStructsAndConstants.h"
#include "common/StringHelper.h"
#include "process/base/IndexEntryV1.h"
#include <regex>
#include <string>

using namespace std;

/**
 * Base class for various storage decision strategies for index entries.
 */
class IndexEntryStorageDecisionStrategy {

public:

    static const int64_t AUTO_DISTANCE = -1;

    /**
     * Call this to see, if the current block can be used as a reference for an index entry.
     *
     * @param indexEntry    The index entry object for which the decision is done.
     * @param position      The reference entry (e.g. the last stored entry) for the new entry.
     * @param blockIsEmpty  Indicates, that no data is in the block referenced by indexEntry
     * @return   true, if the strategy allows the storage of indexEntry
     */
    virtual bool shallStore(IndexEntryV1_S indexEntry, IndexEntryV1_S referenceEntry, bool blockIsEmpty) = 0;

    /**
     * If implemented by the sub class, this might lead to a modification of the behaviour of shallStore. So call this
     * BEFORE you call shallStore.
     * @param filesize  The size of the file for which this strategy is used.
     */
    virtual void useFileSizeForCalculation(int64_t filesize) {};
};

/**
 * Strategy to store and entry for every n'th (when applicable) compressed block applying (if set) a safe distance
 * check. Empty blocks will not be used as a reference point for an index entry.
 */
class BlockDistanceStorageDecisionStrategy : public IndexEntryStorageDecisionStrategy {

protected:

    /**
     * The minimum interval between two stored blocks.
     */
    int blockInterval{0};

    /**
     * Set to true, to enable an extended check, for the Byte-distance between two index entries. If this distance is
     * too small, the strategy will delay storage, until the safe distance is overstepped. See the explanation of
     * shallStore() for more details.
     */
    bool useMinimumByteDistanceCheck{false};

public:

    static const uint DEFAULT_BLOCKINTERVAL = 2048;

    static shared_ptr<BlockDistanceStorageDecisionStrategy> getDefault() {
        return from(AUTO_DISTANCE);
    }

    static shared_ptr<BlockDistanceStorageDecisionStrategy> from(int blockInterval, bool useSafeDistanceCheck = true) {
        return make_shared<BlockDistanceStorageDecisionStrategy>(blockInterval, useSafeDistanceCheck);
    }

    static u_int32_t calculateIndexBlockInterval(int64_t fileSize) {
        int64_t normalizedSize = fileSize / GB;

        if (normalizedSize <= 1)
            return 16;
        if (normalizedSize <= 2)
            return 32;
        if (normalizedSize <= 4)
            return 64;
        if (normalizedSize <= 8)
            return 128;
        if (normalizedSize <= 16)
            return 256;
        if (normalizedSize <= 32)
            return 512;
        if (normalizedSize <= 64)
            return 1024;
        if (normalizedSize <= 128)
            return DEFAULT_BLOCKINTERVAL;
        if (normalizedSize <= 256)
            return 4096;
        return 8192;
    }

    BlockDistanceStorageDecisionStrategy(int blockInterval, bool useMinimumByteDistanceCheck) {
        this->blockInterval = blockInterval;
        this->useMinimumByteDistanceCheck = useMinimumByteDistanceCheck;
        if (this->blockInterval <= 0)
            this->blockInterval = AUTO_DISTANCE;
    }

    int getBlockInterval() {
        return blockInterval;
    }

    /**
     * Only write back every nth entry. As we need the large window / dictionary of 32kb, we'll need to save
     * some space here. Think of large NovaSeq FASTQ files with ~200GB! We can't store approximately 3.6m index
     * entries which would be around 115GB for an index file
     *
     * In addition, empty blocks (blocks containing no data) are skipped until the next valid block occurs
     *
     * Also there is one huge difficulty, e.g. in test files from here:
     * https://trace.ncbi.nlm.nih.gov/Traces/sra/sra.cgi?exp=SRX000001&cmd=search&m=downloads&s=seq
     *  What I found out is, that the checked files contain very small compressed blocks each holding just a few bytes.
     *  To tackle this, we introduce the failsafe distance when writing index entries. So not only the block distance
     *  will be looked at but also the Byte distance to the last written index entry.
     * Why?  In one exemplary case, the file size is approximately 145MB and it contains around 7,35 million compressed
     *  blocks. This results in ca. 20Bytes per block. E.g. storing every 16th block with ~ 32kByte results in a
     *  hopelessly huge index file.
     *
     * It is allowed to have the blockInterval set to -1. In that case it is allowed to automatically
     * set the value by calling useFilesizeForCalculation(). However, if it is still -1, we apply a default value here.
     */
    bool shallStore(IndexEntryV1_S indexEntry, IndexEntryV1_S lastStoredIndexEntry, bool blockIsEmpty) override {
        if (blockInterval == AUTO_DISTANCE)
            blockInterval = DEFAULT_BLOCKINTERVAL;

        if (blockIsEmpty) return false;

        auto blockIndex = indexEntry->blockIndex;
        if (blockIndex == 0) return true;


        auto blockOffset = indexEntry->blockOffsetInRawFile;
        u_int64_t offsetOfLastEntry = 0;
        bool minimumByteDistanceIsReached = true;
        if (useMinimumByteDistanceCheck) {
            u_int64_t minimumByteDistance = static_cast<u_int64_t>(blockInterval) * 16384;
            if (lastStoredIndexEntry)
                offsetOfLastEntry = lastStoredIndexEntry->blockOffsetInRawFile;
            minimumByteDistanceIsReached = (blockOffset - offsetOfLastEntry) > (minimumByteDistance);
        }

        auto previousBlockIndex = lastStoredIndexEntry ? lastStoredIndexEntry->blockIndex : 0;
        return ((blockIndex >= previousBlockIndex + blockInterval) && minimumByteDistanceIsReached);
    }

    void useFileSizeForCalculation(int64_t filesize) override {
        if (blockInterval != AUTO_DISTANCE) return;
        // Distance like for files > 128GB when filesize is unavailabe.
        blockInterval = filesize == 0 ? 2048 : calculateIndexBlockInterval(filesize);
    }
};

/**
 * Storage decision strategy, which looks mainly at the current distance to the last index entry reference point
 * (position in the compressed source file)
 */
class ByteDistanceStorageDecisionStrategy : public IndexEntryStorageDecisionStrategy {

private:
    /**
     * Minimum! distance between two compressed source blocks referenced by an index entry in Byte.
     */
    int64_t minIndexEntryByteDistance{0};

public:

    static const int64_t DEFAULT_MININDEXENTRY_BYTEDISTANCE = 256 * 1024 * 1024; // Can't use extern const here.

    static shared_ptr<ByteDistanceStorageDecisionStrategy> getDefault() {
        return from("1G");
    }

    static shared_ptr<ByteDistanceStorageDecisionStrategy> from(const string &value) {
        return make_shared<ByteDistanceStorageDecisionStrategy>(value);
    }

    static int64_t calculateDistanceBasedOnFileSize(int64_t fileSize) {
        int64_t distance = fileSize / 512;
        return distance <= 256 * kB ? 256 * kB : distance;
    }

    explicit ByteDistanceStorageDecisionStrategy(int64_t minIndexEntryByteDistance) {
        this->minIndexEntryByteDistance = minIndexEntryByteDistance;
        if (this->minIndexEntryByteDistance <= 0)
            this->minIndexEntryByteDistance = AUTO_DISTANCE;
    }

    explicit ByteDistanceStorageDecisionStrategy(const string &minIndexEntryByteDistance)
            : ByteDistanceStorageDecisionStrategy(StringHelper::parseStringValue(minIndexEntryByteDistance)) {}

    int64_t getMinIndexEntryByteDistance() {
        return minIndexEntryByteDistance;
    }

    /**
     * It is allowed to have the minIndexEntryByteDistance set to -1. In that case it is allowed to automatically
     * set the value by calling useFilesizeForCalculation(). However, if it is still -1, we apply a default value here.
     */
    bool shallStore(IndexEntryV1_S indexEntry, IndexEntryV1_S lastStoredIndexEntry, bool blockIsEmpty) override {
        if (minIndexEntryByteDistance == AUTO_DISTANCE)
            minIndexEntryByteDistance = DEFAULT_MININDEXENTRY_BYTEDISTANCE;

        if (blockIsEmpty)
            return false;

        // Obviously the first valid entry, always store it.
        if (!lastStoredIndexEntry)
            return true;

        u_int64_t delta = indexEntry->blockOffsetInRawFile - lastStoredIndexEntry->blockOffsetInRawFile;

        return delta > static_cast<uint64_t>(minIndexEntryByteDistance);
    }

    void useFileSizeForCalculation(int64_t filesize) override {
        if (minIndexEntryByteDistance != AUTO_DISTANCE) return;
        minIndexEntryByteDistance = filesize != 0 ?
                                    calculateDistanceBasedOnFileSize(filesize) :
                                    DEFAULT_MININDEXENTRY_BYTEDISTANCE;
    }
};

#endif //FASTQINDEX_INDEXENTRYSTORAGEDECISIONSTRATEGY_H
