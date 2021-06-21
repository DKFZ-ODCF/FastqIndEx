/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/index/IndexWriter.h"
#include "process/extract/IndexReader.h"
#include "process/io/FileSink.h"
#include "process/io/FileSource.h"
#include "TestResourcesAndFunctions.h"
#include "TestConstants.h"
#include <UnitTest++/UnitTest++.h>
#include <memory>

#include <experimental/filesystem>

using namespace std::experimental::filesystem;
using std::experimental::filesystem::path;
using std::experimental::filesystem::perms;

const char *const SUITE_INDEXWRITER_TESTS = "IndexWriterTestSuite";
const char *const TEST_CREATION_WITH_EXISTING_FILE = "Creation with existing file";
const char *const TEST_CREATION_WITH_MISSING_FILE = "Creation with missing file";
const char *const TEST_CREATION_WITHOUT_WRITE_ACCESS = "Creation with missing write access to the target folder";
const char *const TEST_WRITE_HEADER_TO_NEWLY_OPENED_FILE = "Write header to newly opened file";
const char *const TEST_WRITE_HEADER_TWICE = "Write header a second time";
const char *const TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE = "Write index entry to newly opened file";
const char *const TEST_WRITE_INDEX_TO_END_OF_FILE = "Write index entry at end of file";
const char *const TEST_WRITE_ENTRY_CHECK_HEADER_AFTER_CLOSE = "Write index - delete the writer - check the header for validity.";


SUITE (SUITE_INDEXWRITER_TESTS) {
    TEST (TEST_CREATION_WITH_MISSING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITH_EXISTING_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(make_shared<FileSink>(index));
                CHECK(iw);
    }

    TEST (TEST_CREATION_WITH_EXISTING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITH_EXISTING_FILE);
        path index = res.createEmptyFile(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(make_shared<FileSink>(index));
                CHECK(!iw->tryOpen());
                CHECK(!iw->hasLock());
    }

    TEST (TEST_CREATION_WITHOUT_WRITE_ACCESS) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITHOUT_WRITE_ACCESS);
        path index = res.filePath(INDEX_FILENAME);
        permissions(res.getTestPath(), perms::owner_read | perms::owner_exec);
        auto iw = make_shared<IndexWriter>(make_shared<FileSink>(index));
                CHECK(!iw->tryOpen());
                CHECK(!iw->hasLock());
        permissions(res.getTestPath(), perms::owner_all);
    }

    TEST (TEST_WRITE_HEADER_TO_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_CREATION_WITH_EXISTING_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(make_shared<FileSink>(index));
                CHECK(iw->tryOpen());
        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 1, true);
        bool writeOk = iw->writeIndexHeader(header);
                CHECK(writeOk);
                CHECK(exists(index));
        iw->flush();
                CHECK(file_size(index) == sizeof(IndexHeader));
    }

    TEST (TEST_WRITE_HEADER_TWICE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_WRITE_HEADER_TWICE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(make_shared<FileSink>(index));
                CHECK(iw->tryOpen());

        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 1, true);
                CHECK(iw->writeIndexHeader(header));
                CHECK(!iw->writeIndexHeader(header));
    }

    TEST (TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(make_shared<FileSink>(index));
                CHECK(iw->tryOpen());
        auto entry = make_shared<IndexEntryV1>(0, 0, 0, 0, 0);
                CHECK(!iw->writeIndexEntry(entry));
    }

    TEST (TEST_WRITE_INDEX_TO_END_OF_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE);
        path index = res.filePath(INDEX_FILENAME);
        auto iw = make_shared<IndexWriter>(make_shared<FileSink>(index));
                CHECK(iw->tryOpen());
        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 1, true);
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

    TEST (TEST_WRITE_ENTRY_CHECK_HEADER_AFTER_CLOSE) {
        TestResourcesAndFunctions res(SUITE_INDEXWRITER_TESTS, TEST_WRITE_INDEX_TO_NEWLY_OPENED_FILE);
        path index = res.filePath(INDEX_FILENAME);

        auto iw = new IndexWriter(make_shared<FileSink>(index));
                CHECK(iw->tryOpen());
        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 1, true);
                CHECK(iw->writeIndexHeader(header));
        auto entry = make_shared<IndexEntryV1>(0, 0, 0, 0, 0);
                CHECK(iw->writeIndexEntry(entry));
                CHECK(iw->writeIndexEntry(entry));

        delete iw;

        IndexReader ir(make_shared<FileSource>(index));
                CHECK(ir.tryOpenAndReadHeader());
        auto readHeader = ir.getIndexHeader();
                CHECK(readHeader.numberOfEntries == 2);
    }
}