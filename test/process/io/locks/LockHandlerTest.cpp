/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/locks/PathLockHandler.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *const SUITE_PATH_LOCKHANDLER_TESTS = "PathLockHandler TestSuite";
const char *const TEST_GET_CREATION = "Test creation";
const char *const TEST_IP_OPEN_CLOSE_OPEN = "Test write after read and close, effectively test the unlock op.";
const char *const TEST_IP_OPEN_READ_TWICE = "Test read two times";
const char *const TEST_IP_OPEN_READ_WRITE = "Test tryOpenAndReadHeader of read first, write second";
const char *const TEST_IP_OPEN_WRITE_READ = "Test tryOpenAndReadHeader of write first, read second";
const char *const TEST_IP_OPEN_WRITE_WRITE = "Test tryOpenAndReadHeader of write twice";
const char *const TEST_FQI_FILE = "someTest.fqi";

SUITE (SUITE_PATH_LOCKHANDLER_TESTS) {
    TEST (TEST_GET_CREATION) {
        TestResourcesAndFunctions res(SUITE_PATH_LOCKHANDLER_TESTS, TEST_GET_CREATION);
        auto idx = res.createEmptyFile(TEST_FQI_FILE);
        auto lock = res.createEmptyFile("someTest.fqi~");
        PathLockHandler indexProcessor(idx);
                CHECK_EQUAL(indexProcessor.getIndexFile(), idx);
    }

    TEST (TEST_IP_OPEN_CLOSE_OPEN) {
        TestResourcesAndFunctions res(SUITE_PATH_LOCKHANDLER_TESTS, TEST_IP_OPEN_CLOSE_OPEN);
        auto idx = res.createEmptyFile(TEST_FQI_FILE);
        PathLockHandler ip1(idx);
                CHECK(ip1.readLock());
                CHECK(ip1.hasLock());
        ip1.unlock();
                CHECK(!ip1.hasLock());
                CHECK(ip1.writeLock());
                CHECK(ip1.hasLock());
        ip1.unlock();
    }

    TEST (TEST_IP_OPEN_READ_TWICE) {
        TestResourcesAndFunctions res(SUITE_PATH_LOCKHANDLER_TESTS, TEST_IP_OPEN_READ_TWICE);
        auto idx = res.createEmptyFile(TEST_FQI_FILE);
        PathLockHandler ip1(idx);
                CHECK(ip1.readLock());
                CHECK(ip1.readLock());
    }

    TEST (TEST_IP_OPEN_READ_WRITE) {
        TestResourcesAndFunctions res(SUITE_PATH_LOCKHANDLER_TESTS, TEST_IP_OPEN_READ_WRITE);
        auto idx = res.createEmptyFile(TEST_FQI_FILE);
        PathLockHandler ip1(idx);
                CHECK(ip1.readLock());
                CHECK(ip1.hasLock());
                CHECK(!ip1.writeLock());
    }

    TEST (TEST_IP_OPEN_WRITE_READ) {
        TestResourcesAndFunctions res(SUITE_PATH_LOCKHANDLER_TESTS, TEST_IP_OPEN_READ_WRITE);
        auto idx = res.createEmptyFile(TEST_FQI_FILE);
        PathLockHandler ip1(idx);
                CHECK(ip1.writeLock());
                CHECK(!ip1.readLock());
    }

    TEST (TEST_IP_OPEN_WRITE_WRITE) {
        TestResourcesAndFunctions res(SUITE_PATH_LOCKHANDLER_TESTS, TEST_IP_OPEN_READ_WRITE);
        auto idx = res.createEmptyFile(TEST_FQI_FILE);
        PathLockHandler ip1(idx);
                CHECK(ip1.writeLock());
                CHECK(!ip1.writeLock());
        ip1.unlock();
                CHECK(!ip1.hasLock());
    }
}