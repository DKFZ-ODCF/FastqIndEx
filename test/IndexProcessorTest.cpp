/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/IndexProcessor.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *SUITE_INDEXPROCESSOR_TESTS = "IndexProcessorTestSuite";
const char *TEST_GET_CREATION = "Test creation";
const char *TEST_IP_OPEN_CLOSE_OPEN = "Test write after read and close, effectively test the unlock op.";
const char *TEST_IP_OPEN_READ_TWICE = "Test read two times";
const char *TEST_IP_OPEN_READ_WRITE = "Test tryOpen of read first, write second";
const char *TEST_IP_OPEN_WRITE_READ = "Test tryOpen of write first, read second";
const char *TEST_IP_OPEN_WRITE_WRITE = "Test tryOpen of write twice";

/**
 * We cannot really test interprocess communication (ok, we could somehow but it is not a real unit test anmyore)
 * and have to rely on boost here. But what we can do is the basic locking semantics. So what we do is to create one
 * IndexProcessor on which we will perform the operations sequentially. Luckily, calling e.g.
 * interprocess_shared_mutex.lock() twice will fail and behave like with interprocess communcation.
 */
SUITE (SUITE_INDEXPROCESSOR_TESTS) {
    TEST (TEST_GET_CREATION) {
        TestResourcesAndFunctions res(SUITE_INDEXPROCESSOR_TESTS, TEST_GET_CREATION);
        auto idx = res.createEmptyFile("someTest.idx");
        auto lock = res.createEmptyFile("someTest.idx~");
        IndexProcessor indexProcessor(idx);
                CHECK_EQUAL(indexProcessor.getIndexFile(), idx);
    }

    TEST (TEST_IP_OPEN_CLOSE_OPEN) {
        TestResourcesAndFunctions res(SUITE_INDEXPROCESSOR_TESTS, TEST_IP_OPEN_CLOSE_OPEN);
        auto idx = res.createEmptyFile("someTest.idx");
        IndexProcessor ip1(idx);
                CHECK(ip1.lockForReading());
                CHECK(ip1.hasLock());
        ip1.unlock();
                CHECK(!ip1.hasLock());
                CHECK(ip1.lockForWriting());
                CHECK(ip1.hasLock());
        ip1.unlock();
    }

    TEST (TEST_IP_OPEN_READ_TWICE) {
        TestResourcesAndFunctions res(SUITE_INDEXPROCESSOR_TESTS, TEST_IP_OPEN_READ_TWICE);
        auto idx = res.createEmptyFile("someTest.idx");
        IndexProcessor ip1(idx);
                CHECK(ip1.lockForReading());
                CHECK(ip1.lockForReading());
    }

    TEST (TEST_IP_OPEN_READ_WRITE) {
        TestResourcesAndFunctions res(SUITE_INDEXPROCESSOR_TESTS, TEST_IP_OPEN_READ_WRITE);
        auto idx = res.createEmptyFile("someTest.idx");
        IndexProcessor ip1(idx);
                CHECK(ip1.lockForReading());
                CHECK(ip1.hasLock());
                CHECK(!ip1.lockForWriting());
    }

    TEST (TEST_IP_OPEN_WRITE_READ) {
        TestResourcesAndFunctions res(SUITE_INDEXPROCESSOR_TESTS, TEST_IP_OPEN_READ_WRITE);
        auto idx = res.createEmptyFile("someTest.idx");
        IndexProcessor ip1(idx);
                CHECK(ip1.lockForWriting());
                CHECK(!ip1.lockForReading());
    }

    TEST (TEST_IP_OPEN_WRITE_WRITE) {
        TestResourcesAndFunctions res(SUITE_INDEXPROCESSOR_TESTS, TEST_IP_OPEN_READ_WRITE);
        auto idx = res.createEmptyFile("someTest.idx");
        IndexProcessor ip1(idx);
                CHECK(ip1.lockForWriting());
                CHECK(!ip1.lockForWriting());
        ip1.unlock();
                CHECK(!ip1.hasLock());
    }
}