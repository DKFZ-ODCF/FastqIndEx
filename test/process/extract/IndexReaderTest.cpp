/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/extract/IndexReader.h"
#include "process/index/IndexWriter.h"
#include "process/io/FileSink.h"
#include "process/io/FileSource.h"
#include "TestResourcesAndFunctions.h"
#include "TestConstants.h"
#include <fstream>
#include <memory>
#include <UnitTest++/UnitTest++.h>

using std::experimental::filesystem::perms;

const char *SUITE_INDEXREADER_TESTS = "IndexReaderTestSuite";
const char *TEST_READER_CREATION_WITH_EXISTING_FILE = "Creation with existing file";
const char *TEST_READER_CREATION_WITH_UNREADABLE_FILE = "Creation with unreadable file";
const char *TEST_READER_CREATION_WITH_MISSING_FILE = "Creation with missing file";
const char *TEST_READER_CREATION_WITH_SIZE_MISMATCH = "Creation with file with size mismatch";
const char *TEST_READER_CREATION_WITH_TOO_FEW_ENTRIES = "Creation with file with too few entries";
const char *TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE = "Read header from newly opened file";
const char *TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE = "Read index entry from newly opened file";
const char *TEST_READ_INDEX_FROM_FILE = "Read index entry from file";
const char *TEST_READ_SEVERAL_ENTRIES_FROM_FILE = "Read several index entries from file";
const char *TEST_READ_INDEX_FROM_END_OF_FILE = "Read index entry at end of file";

const size_t BASE_FILE_SIZE = sizeof(IndexEntryV1) + sizeof(IndexHeader);

auto createCharArray(size_t size) {
    shared_ptr<char> arr(new char[size]);
    memset(arr.get(), 0, static_cast<size_t>(size));
    return arr;
}

path writeTestFile(TestResourcesAndFunctions *res, size_t size) {
    path idx = res->createEmptyFile(INDEX_FILENAME);

    ofstream out(idx);

    auto chars = createCharArray(size);

    out.write(chars.get(), size);
    out.flush();
    return idx;
}

// TODO Test for different tryOpenAndReadHeader() problems (e.g. test file with bad index writer version!

// TODO Test for readIndexFile() with and without bad index writer version

SUITE (SUITE_INDEXREADER_TESTS) {
    TEST (TEST_READER_CREATION_WITH_EXISTING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_EXISTING_FILE);
        path idx = TestResourcesAndFunctions::getResource(TEST_INDEX_SMALL);
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(ir->tryOpenAndReadHeader());
                CHECK(ir->getErrorMessages().empty());
                CHECK_EQUAL(ir->getIndicesLeft(), 1);
    }

    TEST (TEST_READER_CREATION_WITH_UNREADABLE_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_UNREADABLE_FILE);
        path idx = res.createEmptyFile(INDEX_FILENAME);

//        permissions(idx, perms::owner_write | perms::owner_exec);
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(!ir->tryOpenAndReadHeader());
    }

    TEST (TEST_READER_CREATION_WITH_MISSING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_MISSING_FILE);
        path idx = res.filePath(INDEX_FILENAME);
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(!ir->tryOpenAndReadHeader());
    }

    TEST (TEST_READER_CREATION_WITH_SIZE_MISMATCH) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_SIZE_MISMATCH);
        path idx = writeTestFile(&res, BASE_FILE_SIZE + 3);
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(!ir->tryOpenAndReadHeader());
    }

    TEST (TEST_READER_CREATION_WITH_TOO_FEW_ENTRIES) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_TOO_FEW_ENTRIES);
        path idx = writeTestFile(&res, sizeof(IndexHeader));
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(!ir->tryOpenAndReadHeader());
    }

    // TODO test for file with wrong indexer.

    TEST (TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE);
        path idx = TestResourcesAndFunctions::getResource(TEST_INDEX_SMALL);
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(ir->tryOpenAndReadHeader());

        auto header = ir->getIndexHeader(); // Will effectively return the already read header.

        int64_t test[62] = {0};

//                CHECK(header.get() != nullptr);
                CHECK_EQUAL(1U, header.indexWriterVersion);
                CHECK_EQUAL(67305985U, header.magicNumber);
                CHECK_ARRAY_EQUAL(test, header.reserved, 59);
    }

    TEST (TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE);
        path idx = TestResourcesAndFunctions::getResource(TEST_INDEX_SMALL);

        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(ir->tryOpenAndReadHeader());

        auto entry = ir->readIndexEntryV1();
                CHECK(entry);
    }

    TEST (TEST_READ_INDEX_FROM_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_FILE);
        path idx = TestResourcesAndFunctions::getResource(TEST_INDEX_LARGE);

        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(ir->tryOpenAndReadHeader());

        auto header = ir->getIndexHeader();

//                CHECK(header);

        Bytef emptyWindow[WINDOW_SIZE]{0};

        auto entry = ir->readIndexEntryV1();
                CHECK(entry);
                CHECK_EQUAL(0U, entry->bits);
                CHECK_EQUAL(0U, entry->offsetToNextLineStart);
                CHECK_EQUAL(10U, entry->blockOffsetInRawFile);
                CHECK_EQUAL(0U, entry->startingLineInEntry);
                CHECK_ARRAY_EQUAL(emptyWindow, entry->dictionary, sizeof(emptyWindow));

        entry = ir->readIndexEntryV1();
                CHECK(entry);
                CHECK_EQUAL(6U, entry->bits);
                CHECK_EQUAL(219567U, entry->blockOffsetInRawFile);
        for (uint64_t i = 0; i < sizeof(emptyWindow); i++) {
                    CHECK(entry->dictionary != nullptr);
        }
    }

    TEST (TEST_READ_SEVERAL_ENTRIES_FROM_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_SEVERAL_ENTRIES_FROM_FILE);
        path idx = TestResourcesAndFunctions::getResource(TEST_INDEX_LARGE);
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(ir->tryOpenAndReadHeader());

        auto header = ir->getIndexHeader();

//                CHECK(header);
                CHECK(ir->readIndexEntryV1());
                CHECK(ir->readIndexEntryV1());
                CHECK(ir->readIndexEntryV1());
                CHECK(ir->readIndexEntryV1());

        // TODO Add again!
//        auto entry = ir->readIndexEntryV1();
//                CHECK(entry);
//                CHECK_EQUAL(7, entry->bits);
//                CHECK_EQUAL(0, entry->offsetToNextLineStart);
//                CHECK_EQUAL(10, entry->blockOffsetInRawFile);
//                CHECK_EQUAL(0, entry->startingLineInEntry);
    }

    TEST (TEST_READ_INDEX_FROM_END_OF_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_END_OF_FILE);
        path idx = TestResourcesAndFunctions::getResource(TEST_INDEX_SMALL);
        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(ir->tryOpenAndReadHeader());

        auto header = ir->getIndexHeader();

//                CHECK(header);
                CHECK_EQUAL(1, ir->getIndicesLeft());
        auto entry1 = ir->readIndexEntryV1();
                CHECK_EQUAL(0, ir->getIndicesLeft());
        auto entry2 = ir->readIndexEntryV1();
                CHECK_EQUAL(0, ir->getIndicesLeft());
                CHECK(entry1);
                CHECK(!entry2); // Test below works? Why is the stream still good? It is too small...
    }

    TEST (TEST_COMBINED_READ_AND_WRITE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_END_OF_FILE);
        path idx = res.filePath(INDEX_FILENAME);
        auto header = make_shared<IndexHeader>(1, sizeof(IndexEntryV1), 2, true);
        auto entry0 = make_shared<IndexEntryV1>(10, 1, 0, 0, 1);
        auto entry1 = make_shared<IndexEntryV1>(10, 3, 0, 0, 0);
        auto entry2 = make_shared<IndexEntryV1>(10, 5, 0, 0, 1);
        auto entry3 = make_shared<IndexEntryV1>(10, 7, 0, 0, 0);

        auto writer = new IndexWriter(make_shared<FileSink>(idx));
        bool open = writer->tryOpen();
                CHECK(open);
        writer->writeIndexHeader(header);
        writer->writeIndexEntry(entry0);
        writer->writeIndexEntry(entry1);
        writer->writeIndexEntry(entry2);
        writer->writeIndexEntry(entry3);

        delete writer;

        auto ir = make_shared<IndexReader>(make_shared<FileSource>(idx));
                CHECK(ir->tryOpenAndReadHeader());
                CHECK_EQUAL(4, ir->getIndicesLeft());

        auto readHeader = ir->getIndexHeader();
        auto readEntry0 = ir->readIndexEntryV1();
        auto readEntry1 = ir->readIndexEntryV1();
        auto readEntry2 = ir->readIndexEntryV1();
        auto readEntry3 = ir->readIndexEntryV1();

                CHECK(*header == readHeader);
                CHECK(*entry0 == *readEntry0);
                CHECK(*entry1 == *readEntry1);
                CHECK(*entry2 == *readEntry2);
                CHECK(*entry3 == *readEntry3);
    }
}