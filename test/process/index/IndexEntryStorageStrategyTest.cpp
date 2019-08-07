/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/CommonStructsAndConstants.h"
#include "process/index/IndexEntryStorageStrategy.h"
#include <UnitTest++/UnitTest++.h>
#include <iostream>

const char *const INDEXENTRY_STORAGESTRATEGY_SUITE_TESTS = "IndexEntryStorageStrategyTests";
const char *const TEST_BLOCKSTRATEGY_CALCULATE_BLOCK_INTERVAL = "Test calculateBlockInterval";
const char *const TEST_BLOCKSTRATEGY_SHALLSTORE = "Test calculateBlockInterval";
const char *const TEST_BYTESTRATEGY_PARSESTRINGVALUE = "Test parseStringValue";
const char *const TEST_BYTESTRATEGY_CONSTRUCT = "Test construction";
const char *const TEST_BYTESTRATEGY_SHALLSTORE = "Test parseStringValue";

SUITE (INDEXENTRY_STORAGESTRATEGY_SUITE_TESTS) {

    TEST (TEST_BLOCKSTRATEGY_CALCULATE_BLOCK_INTERVAL) {
                CHECK_EQUAL(16, BlockDistanceStorageStrategy::calculateIndexBlockInterval(1 * GB));
                CHECK_EQUAL(32, BlockDistanceStorageStrategy::calculateIndexBlockInterval(2 * GB));
                CHECK_EQUAL(64, BlockDistanceStorageStrategy::calculateIndexBlockInterval(4 * GB));
                CHECK_EQUAL(128, BlockDistanceStorageStrategy::calculateIndexBlockInterval(8 * GB));
                CHECK_EQUAL(512, BlockDistanceStorageStrategy::calculateIndexBlockInterval(20 * GB));
                CHECK_EQUAL(512, BlockDistanceStorageStrategy::calculateIndexBlockInterval(30 * GB));
                CHECK_EQUAL(1024, BlockDistanceStorageStrategy::calculateIndexBlockInterval(40 * GB));
                CHECK_EQUAL(2048, BlockDistanceStorageStrategy::calculateIndexBlockInterval(80 * GB));
                CHECK_EQUAL(2048, BlockDistanceStorageStrategy::calculateIndexBlockInterval(120 * GB));
                CHECK_EQUAL(4096, BlockDistanceStorageStrategy::calculateIndexBlockInterval(160 * GB));
                CHECK_EQUAL(4096, BlockDistanceStorageStrategy::calculateIndexBlockInterval(200 * GB));
                CHECK_EQUAL(8192, BlockDistanceStorageStrategy::calculateIndexBlockInterval(300 * GB));
        //Maximum value
                CHECK_EQUAL(8192, BlockDistanceStorageStrategy::calculateIndexBlockInterval(430 * GB));
                CHECK_EQUAL(8192, BlockDistanceStorageStrategy::calculateIndexBlockInterval(1630 * GB));
    }

    TEST (TEST_BLOCKSTRATEGY_SHALLSTORE) {
        BlockDistanceStorageStrategy strat(16, true);
        auto ie0 = make_shared<IndexEntryV1>(0, 0, 16, 32, 0);
        auto ie1 = make_shared<IndexEntryV1>(0, 15, 19, 512 * MB, 3);
        auto ie2 = make_shared<IndexEntryV1>(0, 16, 19, 1039 * MB, 3);
        auto ie3 = make_shared<IndexEntryV1>(0, 17, 19, 1039 * MB, 3);
        auto ie4 = make_shared<IndexEntryV1>(0, 32, 32, 2048 * MB, 7);
                CHECK(strat.shallStore(ie0, 0, false));
                CHECK(!strat.shallStore(ie1, 15, false)); // Not in distance
                CHECK(!strat.shallStore(ie2, 16, true));  // Empty
                CHECK(strat.shallStore(ie3, 17, false));  // Not in distance but with postpone flag set
                CHECK(strat.shallStore(ie4, 32, false));
    }

    TEST (TEST_BLOCKSTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_SET_DISTANCE) {
        auto strat = BlockDistanceStorageStrategy::from(75, true);
        strat->useFilesizeForCalculation(4 * GB);
                CHECK_EQUAL(75, strat->getBlockInterval());
    }

    TEST (TEST_BLOCKSTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_NO_DISTANCE) {
        auto strat = BlockDistanceStorageStrategy::from(-1, true);
        strat->useFilesizeForCalculation(4 * GB);
                CHECK_EQUAL(64, strat->getBlockInterval());
    }

    TEST (TEST_BYTESTRATEGY_PARSESTRINGVALUE) {
        // Invalid values first => Will result in -1
                CHECK_EQUAL(-1, ByteDistanceStorageStrategy::parseStringValue(""));
                CHECK_EQUAL(-1, ByteDistanceStorageStrategy::parseStringValue("ab"));
                CHECK_EQUAL(-1, ByteDistanceStorageStrategy::parseStringValue("3a"));
                CHECK_EQUAL(-1, ByteDistanceStorageStrategy::parseStringValue("3.0k"));

        // Valid values
                CHECK_EQUAL(3 * MB, ByteDistanceStorageStrategy::parseStringValue("3"));

                CHECK_EQUAL(3 * kB, ByteDistanceStorageStrategy::parseStringValue("3k"));
                CHECK_EQUAL(3 * kB, ByteDistanceStorageStrategy::parseStringValue("3K"));
                CHECK_EQUAL(3 * MB, ByteDistanceStorageStrategy::parseStringValue("3m"));
                CHECK_EQUAL(3 * MB, ByteDistanceStorageStrategy::parseStringValue("3M"));

                CHECK_EQUAL(3 * GB, ByteDistanceStorageStrategy::parseStringValue("3g"));
                CHECK_EQUAL(3 * GB, ByteDistanceStorageStrategy::parseStringValue("3G"));
                CHECK_EQUAL(3 * TB, ByteDistanceStorageStrategy::parseStringValue("3t"));
                CHECK_EQUAL(3 * TB, ByteDistanceStorageStrategy::parseStringValue("3T"));
    }

    TEST (TEST_BYTESTRATEGY_CALCULATE_DISTANCE) {
        u_int64_t values[] = {
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

        u_int64_t expected[] = {
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
                 << ByteDistanceStorageStrategy::calculateDistanceBasedOnFileSize(values[i]) << "\n";
                    CHECK_EQUAL(expected[i], ByteDistanceStorageStrategy::calculateDistanceBasedOnFileSize(values[i]));
        }

    }

    TEST (TEST_BYTESTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_SET_DISTANCE) {
        // distance was already set.
        auto strat = ByteDistanceStorageStrategy::getDefault();
        strat->useFilesizeForCalculation(4 * GB);
                CHECK_EQUAL(1 * GB, strat->getMinIndexEntryByteDistance());
    }

    TEST (TEST_BYTESTRATEGY_USE_FILESIZE_FOR_CALCULATION_WITH_NO_DISTANCE) {
        // distance was -1
        auto strat = make_shared<ByteDistanceStorageStrategy>(-1);
        strat->useFilesizeForCalculation(4 * GB);
                CHECK_EQUAL(8 * MB, strat->getMinIndexEntryByteDistance());
    }

    TEST (TEST_BYTESTRATEGY_CONSTRUCT) {
        // Will call from and the constructor with 1G
        auto strat = ByteDistanceStorageStrategy::getDefault();
                CHECK(strat->getMinIndexEntryByteDistance() == GB);

        strat = make_shared<ByteDistanceStorageStrategy>(-1);
                CHECK(strat->getMinIndexEntryByteDistance() == -1);

        strat->useFilesizeForCalculation(400 * MB);
    }

    TEST (TEST_BYTESTRATEGY_SHALLSTORE) {
        ByteDistanceStorageStrategy strat("1G");
        auto ie0 = make_shared<IndexEntryV1>(0, 0, 16, 32, 0);
        auto ie1 = make_shared<IndexEntryV1>(0, 1, 19, 512 * MB, 3);
        auto ie2 = make_shared<IndexEntryV1>(0, 2, 19, 1039 * MB, 3);
        auto ie3 = make_shared<IndexEntryV1>(0, 3, 32, 1040 * MB, 7);
                CHECK(strat.shallStore(ie0, 0, false));
                CHECK(!strat.shallStore(ie1, 0, false)); // Not in byte distance
                CHECK(!strat.shallStore(ie2, 0, true));  // Empty
                CHECK(strat.shallStore(ie3, 0, false));
    }
}