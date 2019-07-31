/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

const char *const PATH_SOURCE_TEST_SUITE = "Test suite for the PathSource class";
const char *const PATH_SOURCE_CONSTRUCT = "Test construction and checkPremises()";
const char *const PATH_SOURCE_OPEN = "Test open and close";
const char *const PATH_SOURCE_OPENLOCKED = "Test open and close with lock - unlock";
const char *const PATH_SOURCE_AQUIRELOCK_LATER = "Test suite for the PathSource class";
const char *const PATH_SOURCE_READ_TELL_SEEK = "Test suite for the PathSource class";

#include "process/io/PathSource.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

SUITE (PATH_SOURCE_TEST_SUITE) {
    TEST (PATH_SOURCE_CONSTRUCT) {
        TestResourcesAndFunctions res(PATH_SOURCE_TEST_SUITE, PATH_SOURCE_CONSTRUCT);

        auto ps = res.getResource(TEST_FASTQ_SMALL);
        PathSource p(ps);
                CHECK(p.checkPremises());
                CHECK(p.getPath() == ps);
                CHECK(p.exists());
                CHECK(!p.hasLock());
                CHECK(!p.isOpen());
                CHECK(p.isGood());
                CHECK(!p.isBad());
                CHECK(!p.eof());
                CHECK(p.isFile());
                CHECK(!p.isStream());
                CHECK(p.toString() == ps.string());
                CHECK(p.size() > 0);
                CHECK(p.tell() == 0);
                CHECK(p.getErrorMessages().empty());
                CHECK(p.canRead());
                CHECK(!p.canWrite());
    }


    TEST (PATH_SOURCE_OPEN) {
        TestResourcesAndFunctions res(PATH_SOURCE_TEST_SUITE, PATH_SOURCE_OPEN);

        auto ps = res.getResource(TEST_FASTQ_SMALL);
        PathSource p(ps);

        p.open();
                CHECK(p.isOpen());
        p.close();
                CHECK(!p.isOpen());
    }

    TEST (PATH_SOURCE_OPENLOCKED) {
        TestResourcesAndFunctions res(PATH_SOURCE_TEST_SUITE, PATH_SOURCE_OPENLOCKED);
        auto ps = res.getResource(TEST_FASTQ_SMALL);
        PathSource p(ps);
                CHECK(!p.isOpen());

        p.openWithReadLock();

                CHECK(p.isOpen());
                CHECK(p.hasLock());

        p.close();
                CHECK(!p.isOpen());
                CHECK(!p.hasLock());
    }

    TEST (PATH_SOURCE_AQUIRELOCK_LATER) {
        TestResourcesAndFunctions res(PATH_SOURCE_TEST_SUITE, PATH_SOURCE_AQUIRELOCK_LATER);

        auto ps = res.getResource(TEST_FASTQ_SMALL);
        PathSource p(ps);
                CHECK(!p.isOpen());

        p.open();
                CHECK(!p.hasLock());
                CHECK(p.isOpen());

        p.openWithReadLock();
                CHECK(p.hasLock());
                CHECK(p.isOpen());
    }

    TEST (PATH_SOURCE_READ_TELL_SEEK) {
        TestResourcesAndFunctions res(PATH_SOURCE_TEST_SUITE, PATH_SOURCE_READ_TELL_SEEK);

        auto ps = res.getResource(TEST_FASTQ_SMALL);
        PathSource p(ps);
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