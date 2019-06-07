/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/IndexWriter.h"
#include "TestResourcesAndFunctions.h"
#include "TestConstants.h"
#include <UnitTest++/UnitTest++.h>
#include <memory>

#include <experimental/filesystem>

using namespace std::experimental::filesystem;
using std::experimental::filesystem::path;
using std::experimental::filesystem::perms;

const char *SUITE_INDEXWRITER_TESTS = "IndexWriterTestSuite";
const char *TEST_CREATION_WITH_EXISTING_FILE = "Creation with existing file";
const char *TEST_CREATION_WITH_MISSING_FILE = "Creation with missing file";
const char *TEST_CREATION_WITHOUT_WRITE_ACCESS = "Creation with missing write access to the target folder";
const char *TEST_WRITE_HEADER_TO_NEWLY_OPENED_FILE = "Write header to newly opened file";
const char *TEST_WRITE_HEADER_TWICE = "Write header a second time";
const char *TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE = "Write index entry to newly opened file";
const char *TEST_WRITE_INDEX_TO_END_OF_FILE = "Write index entry at end of file";


SUITE (SUITE_INDEXWRITER_TESTS) {
    TEST (TEST_CREATION_WITH_MISSING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITH_EXISTING_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(index);
                CHECK(iw);
    }

    TEST (TEST_CREATION_WITH_EXISTING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITH_EXISTING_FILE);
        path index = res.createEmptyFile(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(index);
                CHECK(!iw->tryOpen());
                CHECK(!iw->hasLock());
    }

    TEST (TEST_CREATION_WITHOUT_WRITE_ACCESS) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITHOUT_WRITE_ACCESS);
        path index = res.filePath(INDEX_FILENAME);
        permissions(res.getTestPath(), perms::owner_read | perms::owner_exec);
        auto iw = make_shared<IndexWriter>(index);
                CHECK(!iw->tryOpen());
                CHECK(!iw->hasLock());
        permissions(res.getTestPath(), perms::owner_all);
    }

    TEST (TEST_WRITE_HEADER_TO_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITH_EXISTING_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(index);
                CHECK(iw->tryOpen());
        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 1);
        bool writeOk = iw->writeIndexHeader(header);
                CHECK(writeOk);
                CHECK(exists(index));
        iw->flush();
                CHECK(file_size(index) == sizeof(IndexHeader));
    }

    TEST (TEST_WRITE_HEADER_TWICE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_WRITE_HEADER_TWICE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(index);
                CHECK(iw->tryOpen());

        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 1);
                CHECK(iw->writeIndexHeader(header));
                CHECK(!iw->writeIndexHeader(header));
    }

    TEST (TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(index);
                CHECK(iw->tryOpen());
        auto entry = make_shared<IndexEntryV1>(0, 0, 0, 0, 0);
                CHECK(!iw->writeIndexEntry(entry));
    }

    TEST (TEST_WRITE_INDEX_TO_END_OF_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(index);
                CHECK(iw->tryOpen());
        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 1);
        bool writeOk = iw->writeIndexHeader(header);
        auto entry = make_shared<IndexEntryV1>(0, 0, 0, 0, 0);
        bool writeOk1 = iw->writeIndexEntry(entry);
        bool writeOk2 = iw->writeIndexEntry(entry);
        iw->flush();

                CHECK(writeOk);
                CHECK(writeOk1);
                CHECK(writeOk2);
                CHECK(file_size(index) == sizeof(IndexHeader) + 2 * sizeof(IndexEntryV1));

    }

}