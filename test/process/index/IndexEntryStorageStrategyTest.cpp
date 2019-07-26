/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/index/IndexEntryStorageStrategy.h"
#include <UnitTest++/UnitTest++.h>

const char *const INDEXENTRY_STORAGESTRATEGY_SUITE_TESTS = "IndexEntryStorageStrategyTests";
const char *const TEST_CALCULATE_BLOCK_INTERVAL = "Test calculateBlockInterval";
const char *const TEST_BYTESTRATEGY_PARSESTRINGVALUE = "Test parseStringValue";

SUITE (INDEXENTRY_STORAGESTRATEGY_SUITE_TESTS) {

    TEST (TEST_CALCULATE_BLOCK_INTERVAL) {
        const u_int64_t GB = 1024 * 1024 * 1024;
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

    TEST (TEST_BYTESTRATEGY_PARSESTRINGVALUE) {
        // Invalid values first
                CHECK_EQUAL(1073741824, ByteDistanceStorageStrategy::parseStringValue(""));
                CHECK_EQUAL(1073741824, ByteDistanceStorageStrategy::parseStringValue("ab"));
                CHECK_EQUAL(1073741824, ByteDistanceStorageStrategy::parseStringValue("3a"));
                CHECK_EQUAL(1073741824, ByteDistanceStorageStrategy::parseStringValue("3.0k"));

        // Default and regular values
                CHECK_EQUAL(3 * 1024 * 1024, ByteDistanceStorageStrategy::parseStringValue("3"));

                CHECK_EQUAL(3 * 1024, ByteDistanceStorageStrategy::parseStringValue("3k"));
                CHECK_EQUAL(3 * 1024, ByteDistanceStorageStrategy::parseStringValue("3K"));
                CHECK_EQUAL(3 * 1024 * 1024, ByteDistanceStorageStrategy::parseStringValue("3m"));
                CHECK_EQUAL(3 * 1024 * 1024, ByteDistanceStorageStrategy::parseStringValue("3M"));

        // CHECK_EQUAL won't work here!
                CHECK_EQUAL(((u_int64_t) 3) * 1024 * 1024 * 1024,
                            ByteDistanceStorageStrategy::parseStringValue("3g"));
                CHECK_EQUAL(((u_int64_t) 3) * 1024 * 1024 * 1024,
                            ByteDistanceStorageStrategy::parseStringValue("3G"));
                CHECK_EQUAL(((u_int64_t) 3) * 1024 * 1024 * 1024 * 1024,
                            ByteDistanceStorageStrategy::parseStringValue("3t"));
                CHECK_EQUAL(((u_int64_t) 3) * 1024 * 1024 * 1024 * 1024,
                            ByteDistanceStorageStrategy::parseStringValue("3T"));
    }
}