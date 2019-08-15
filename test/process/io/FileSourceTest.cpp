/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

const char *const FILE_SOURCE_TEST_SUITE = "Test suite for the FileSource class";
const char *const FILE_SOURCE_CONSTRUCT = "Test construction and fulfillsPremises()";
const char *const FILE_SOURCE_OPEN = "Test open and close";
const char *const FILE_SOURCE_OPENLOCKED = "Test open and close with lock - unlock";
const char *const FILE_SOURCE_AQUIRELOCK_LATER = "Test suite for the FileSource class";
const char *const FILE_SOURCE_READ_TELL_SEEK = "Test suite for the FileSource class";

#include "process/io/FileSource.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

SUITE (FILE_SOURCE_TEST_SUITE) {
    TEST (FILE_SOURCE_CONSTRUCT) {
        TestResourcesAndFunctions res(FILE_SOURCE_TEST_SUITE, FILE_SOURCE_CONSTRUCT);

        auto ps = TestResourcesAndFunctions::getResource(TEST_FASTQ_SMALL);
        FileSource p(ps);
                CHECK(p.fulfillsPremises());
                CHECK(p.getPath() == ps);
                CHECK(p.exists());
                CHECK(!p.hasLock());
                CHECK(!p.isOpen());
                CHECK(p.isGood());
                CHECK(!p.isBad());
                CHECK(p.eof());
                CHECK(p.isFile());
                CHECK(!p.isStream());
                CHECK(p.toString() == ps.string());
                CHECK(p.size() > 0);
                CHECK(p.tell() == 0);
                CHECK(p.getErrorMessages().empty());
                CHECK(p.canRead());
                CHECK(!p.canWrite());
    }


    TEST (FILE_SOURCE_OPEN) {
        TestResourcesAndFunctions res(FILE_SOURCE_TEST_SUITE, FILE_SOURCE_OPEN);

        auto ps = TestResourcesAndFunctions::getResource(TEST_FASTQ_SMALL);
        FileSource p(ps);

        p.open();
                CHECK(p.isOpen());
        p.close();
                CHECK(!p.isOpen());
    }

    TEST (FILE_SOURCE_OPENLOCKED) {
        TestResourcesAndFunctions res(FILE_SOURCE_TEST_SUITE, FILE_SOURCE_OPENLOCKED);
        auto ps = TestResourcesAndFunctions::getResource(TEST_FASTQ_SMALL);
        FileSource p(ps);
                CHECK(!p.isOpen());

        p.openWithReadLock();

                CHECK(p.isOpen());
                CHECK(p.hasLock());

        p.close();
                CHECK(!p.isOpen());
                CHECK(!p.hasLock());
    }

    TEST (FILE_SOURCE_AQUIRELOCK_LATER) {
        TestResourcesAndFunctions res(FILE_SOURCE_TEST_SUITE, FILE_SOURCE_AQUIRELOCK_LATER);

        auto ps = TestResourcesAndFunctions::getResource(TEST_FASTQ_SMALL);
        FileSource p(ps);
                CHECK(!p.isOpen());

        p.open();
                CHECK(!p.hasLock());
                CHECK(p.isOpen());

        p.openWithReadLock();
                CHECK(p.hasLock());
                CHECK(p.isOpen());
    }

    TEST (FILE_SOURCE_READ_TELL_SEEK) {
        TestResourcesAndFunctions res(FILE_SOURCE_TEST_SUITE, FILE_SOURCE_READ_TELL_SEEK);

        auto ps = TestResourcesAndFunctions::getResource(TEST_FASTQ_SMALL);
        FileSource p(ps);
        p.open();

        Bytef a[256]{0};
        p.read(a, sizeof(a));

                CHECK(p.tell() == sizeof(a));
        p.seek(128, true);
                CHECK(p.tell() == 128);
        p.seek(100, false);
                CHECK(p.tell() == 228);
        p.rewind(100);
                CHECK(p.tell() == 128);

        p.seek(p.size() + 1, true);
                CHECK(p.eof());
    }
}