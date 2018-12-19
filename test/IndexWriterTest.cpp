/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "../src/Runner.h"
#include "../src/Indexer.h"
#include "../src/IndexerRunner.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const string SUITE_INDEXREADER_TESTS = "IndexReaderTestSuite"
const string TEST_CREATION_WITH_EXISTING_FILE = "Creation with existing file"
const string TEST_CREATION_WITH_EMPTY_FILE = "Creation with empty file"
const string TEST_CREATION_WITH_SIZE_MISMATCH = "Creation with file with size mismatch"
const string TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE = "Read header from newly opened file"
const string TEST_READ_HEADER_TWICE = "Read header a second time"
const string TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE = "Read index entry from newly opened file"
const string TEST_READ_INDEX_FROM_FILE = "Read index entry from file"
const string TEST_READ_INDEX_FROM_END_OF_FILE = "Read index entry at end of file"

SUITE (SUITE_INDEXREADER_TESTS) {
    TEST (TEST_CREATION_WITH_EXISTING_FILE) {
        TestResourcesAndFunctions(SUITE_INDEXREADER_TESTS, TEST_CREATION_WITH_EXISTING_FILE)

                CHECK(false);
    }


    TEST (TEST_CREATION_WITH_EMPTY_FILE) {
                CHECK(false);
    }

    TEST (TEST_CREATION_WITH_SIZE_MISMATCH) {
                CHECK(false);
    }

    TEST (TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE) {
                CHECK(false);
    }

    TEST (TEST_READ_HEADER_TWICE) {
                CHECK(false);
    }

    TEST (TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE) {
                CHECK(false);
    }

    TEST (TEST_READ_INDEX_FROM_FILE) {
                CHECK(false);
    }

    TEST (TEST_READ_INDEX_FROM_END_OF_FILE) {
                CHECK(false);
    }
}