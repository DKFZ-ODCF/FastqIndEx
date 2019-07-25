/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/ConsoleSink.h"
#include <UnitTest++/UnitTest++.h>

const char *const SUITE_CONSOLESINK_TESTS = "Test suite for ConsoleSink class";
const char *const TEST_STATIC_CREATE_COUT = "Test ::create() and construct std::cout";
const char *const TEST_STATIC_CREATE_CERR = "Test ::create() and construct std::cerr";

SUITE (SUITE_CONSOLESINK_TESTS) {
    TEST (TEST_STATIC_CREATE_COUT) {
        auto sink = ConsoleSink::create();
                CHECK(sink->toString() == "cout");
                CHECK(sink->checkPremises());
                CHECK(sink->isOpen());
                CHECK(sink->close()); // Will not actually close the sink.
                CHECK(sink->tell() == 0);
                CHECK(sink->seek(0, false) == 0);
                CHECK(sink->seek(0, true) == 0);
                CHECK(sink->rewind(10) == 0);
                CHECK(sink->skip(10) == 0);
                CHECK(sink->exists());
                CHECK(sink->isGood());
                CHECK(!sink->isBad());
                CHECK(!sink->eof());
                CHECK(sink->hasLock());
                CHECK(sink->unlock()); // Will not actually unlock the sink.
                CHECK(sink->size() == 0);
                CHECK(sink->canWrite());
                CHECK(!sink->canRead());
                CHECK(sink->isStream());
                CHECK(!sink->isFile());
                CHECK(!sink->isSymlink());
    }

    TEST (TEST_STATIC_CREATE_CERR) {
        auto sink = ConsoleSink::create(ConsoleSinkType::CERR);
        sink->toString() == "cerr";
                CHECK(sink->checkPremises());
                CHECK(sink->isOpen());
                CHECK(sink->close()); // Will not actually close the sink.
                CHECK(sink->tell() == 0);
                CHECK(sink->seek(0, false) == 0);
                CHECK(sink->seek(0, true) == 0);
                CHECK(sink->rewind(10) == 0);
                CHECK(sink->skip(10) == 0);
                CHECK(sink->exists());
                CHECK(sink->isGood());
                CHECK(!sink->isBad());
                CHECK(!sink->eof());
                CHECK(sink->hasLock());
                CHECK(sink->unlock()); // Will not actually unlock the sink.
                CHECK(sink->size() == 0);
                CHECK(sink->canWrite());
                CHECK(!sink->canRead());
                CHECK(sink->isStream());
                CHECK(!sink->isFile());
                CHECK(!sink->isSymlink());
    }
}