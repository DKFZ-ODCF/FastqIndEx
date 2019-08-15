/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/CommonStructsAndConstants.h"
#include "process/index/IndexEntryStorageDecisionStrategy.h"
#include <UnitTest++/UnitTest++.h>
#include <iostream>

const char *const INDEXENTRY_STORAGESTRATEGY_SUITE_TESTS = "IndexEntryStorageStrategyTests";
const char *const TEST_BLOCKSTRATEGY_CALCULATE_BLOCK_INTERVAL = "Test calculateBlockInterval";
const char *const TEST_BLOCKSTRATEGY_SHALLSTORE = "Test calculateBlockInterval";
const char *const TEST_BYTESTRATEGY_PARSESTRINGVALUE = "Test parseStringValue";
const char *const TEST_BYTESTRATEGY_CONSTRUCT = "Test construction";
const char *const TEST_BYTESTRATEGY_SHALLSTORE = "Test shallStore with a valid byte distances";
const char *const TEST_BYTESTRATEGY_SHALLSTORE_INVALID_BYTEDISTANCE = "Test shallStore with a byte distance of -1";

SUITE (INDEXENTRY_STORAGESTRATEGY_SUITE_TESTS) {

    TEST (TEST_BLOCKSTRATEGY_CALCULATE_BLOCK_INTERVAL) {
                CHECK_EQUAL(16U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(1 * GB));
                CHECK_EQUAL(32U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(2 * GB));
                CHECK_EQUAL(64U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(4 * GB));
                CHECK_EQUAL(128U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(8 * GB));
                CHECK_EQUAL(512U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(20 * GB));
                CHECK_EQUAL(512U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(30 * GB));
                CHECK_EQUAL(1024U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(40 * GB));
                CHECK_EQUAL(2048U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(80 * GB));
                CHECK_EQUAL(2048U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(120 * GB));
                CHECK_EQUAL(4096U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(160 * GB));
                CHECK_EQUAL(4096U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(200 * GB));
                CHECK_EQUAL(8192U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(300 * GB));
        //Maximum value
                CHECK_EQUAL(8192U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(430 * GB));
                CHECK_EQUAL(8192U, BlockDistanceStorageDecisionStrategy::calculateIndexBlockInterval(1630 * GB));
    }

    TEST (TEST_BLOCKSTRATEGY_SHALLSTORE) {
        BlockDistanceStorageDecisionStrategy strat(16, true);
        auto ie0 = IndexEntryV1::from(0, 0, 16, 32, 0);
        auto ie1 = IndexEntryV1::from(0, 15, 19, 512 * MB, 3);
        auto ie2 = IndexEntryV1::from(0, 16, 19, 1039 * MB, 3);
        auto ie3 = IndexEntryV1::from(0, 17, 19, 1039 * MB, 3);
        auto ie4 = IndexEntryV1::from(0, 32, 32, 2048 * MB, 7);
                CHECK(strat.shallStore(ie0, shared_ptr<IndexEntryV1>(), false));
                CHECK(!strat.shallStore(ie1, ie0, false)); // Not in distance
                CHECK(!strat.shallStore(ie2, ie0, true));  // Empty
                CHECK(strat.shallStore(ie3, ie0, false));  // Not in distance but with postpone flag set
                CHECK(strat.shallStore(ie4, ie1, false));
    }

    TEST (TEST_BLOCKSTRATEGY_SHALLSTORE_INVALID_BLOCKDISTANCE) {
        BlockDistanceStorageDecisionStrategy strat(-1, true);
        auto ie0 = IndexEntryV1::from(0, 0, 16, 32, 0);
                CHECK(strat.shallStore(ie0, shared_ptr<IndexEntryV1>(), false));
                CHECK(strat.getBlockInterval() == BlockDistanceStorageDecisionStrategy::DEFAULT_BLOCKINTERVAL);
    }

    TEST (TEST_BLOCKSTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_SET_DISTANCE) {
        auto strat = BlockDistanceStorageDecisionStrategy::from(75, true);
        strat->useFileSizeForCalculation(4 * GB);
                CHECK_EQUAL(75, strat->getBlockInterval());
    }

    TEST (TEST_BLOCKSTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_NO_DISTANCE) {
        auto strat = BlockDistanceStorageDecisionStrategy::from(-1, true);
        strat->useFileSizeForCalculation(4 * GB);
                CHECK_EQUAL(64, strat->getBlockInterval());
    }

    TEST (TEST_BYTESTRATEGY_CALCULATE_DISTANCE) {
        int64_t values[] = {
                128 * kB,
                490 * kB,
                700 * kB,
                001 * MB,
                4 * MB,
                8 * MB,
                16 * MB,
                64 * MB,
                130 * MB,
                260 * MB,
                300 * MB,
                900 * MB,
                2 * GB,
                32 * GB,
                128 * GB,
                512 * GB,
        };

        int64_t expected[] = {
                256 * kB,
                256 * kB,
                256 * kB,
                256 * kB,
                256 * kB,
                256 * kB,
                256 * kB,
                256 * kB,
                266240,
                values[9] / 512,
                values[10] / 512,
                values[11] / 512,
                4 * MB,
                64 * MB,
                256 * MB,
                1 * GB,
        };

        for (int i = 0; i < 14; i++) {
            cout << "Expected: " << expected[i] << " / Val : "
                 << ByteDistanceStorageDecisionStrategy::calculateDistanceBasedOnFileSize(values[i]) << "\n";
                    CHECK_EQUAL(expected[i], ByteDistanceStorageDecisionStrategy::calculateDistanceBasedOnFileSize(values[i]));
        }

    }

    TEST (TEST_BYTESTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_SET_DISTANCE) {
        // distance was already set.
        auto strat = ByteDistanceStorageDecisionStrategy::getDefault();
        strat->useFileSizeForCalculation(4 * GB);
                CHECK_EQUAL(1 * GB, strat->getMinIndexEntryByteDistance());
    }

    TEST (TEST_BYTESTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_NO_DISTANCE) {
        // distance was -1
        auto strat = make_shared<ByteDistanceStorageDecisionStrategy>(-1);
        strat->useFileSizeForCalculation(4 * GB);
                CHECK_EQUAL(8 * MB, strat->getMinIndexEntryByteDistance());
    }

    TEST (TEST_BYTESTRATEGY_CONSTRUCT) {
        // Will call from and the constructor with 1G
        auto strat = ByteDistanceStorageDecisionStrategy::getDefault();
                CHECK(strat->getMinIndexEntryByteDistance() == GB);

        strat = make_shared<ByteDistanceStorageDecisionStrategy>(-1);
                CHECK(strat->getMinIndexEntryByteDistance() == -1);

        strat->useFileSizeForCalculation(400 * MB);

                CHECK_EQUAL(-1, ByteDistanceStorageDecisionStrategy(-5).getMinIndexEntryByteDistance());
                CHECK_EQUAL(-1, ByteDistanceStorageDecisionStrategy(0).getMinIndexEntryByteDistance());
                CHECK_EQUAL(1, ByteDistanceStorageDecisionStrategy(1).getMinIndexEntryByteDistance());
    }

    TEST (TEST_BYTESTRATEGY_SHALLSTORE) {
        ByteDistanceStorageDecisionStrategy strat("1G");
        auto ie0 = IndexEntryV1::from(0, 0, 0, 10, 0);
        auto ie1 = IndexEntryV1::from(0, 1, 19, 512 * MB, 3);
        auto ie2 = IndexEntryV1::from(0, 2, 19, 1039 * MB, 3);
        auto ie3 = IndexEntryV1::from(0, 3, 32, 1040 * MB, 7);
                CHECK(strat.shallStore(ie0, shared_ptr<IndexEntryV1>(), false));
                CHECK(!strat.shallStore(ie1, ie0, false)); // Not in byte distance
                CHECK(!strat.shallStore(ie2, ie0, true));  // Empty
                CHECK(strat.shallStore(ie3, ie0, false));
    }

    TEST (TEST_BYTESTRATEGY_SHALLSTORE_INVALID_BYTEDISTANCE) {
        ByteDistanceStorageDecisionStrategy strat(-1);

        auto ie0 = IndexEntryV1::from(0, 0, 16, 32, 0);
                CHECK(strat.shallStore(ie0, shared_ptr<IndexEntryV1>(), false));
                CHECK(strat.getMinIndexEntryByteDistance() ==
                      ByteDistanceStorageDecisionStrategy::DEFAULT_MININDEXENTRY_BYTEDISTANCE);
    }
}