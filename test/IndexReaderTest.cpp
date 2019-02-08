/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "../src/IndexReader.h"
#include "../src/IndexWriter.h"
#include "TestResourcesAndFunctions.h"
#include "TestConstants.h"
#include <UnitTest++/UnitTest++.h>
#include <boost/make_shared.hpp>

const char *SUITE_INDEXREADER_TESTS = "IndexReaderTestSuite";
const char *TEST_READER_CREATION_WITH_EXISTING_FILE = "Creation with existing file";
const char *TEST_READER_CREATION_WITH_UNREADABLE_FILE = "Creation with unreadable file";
const char *TEST_READER_CREATION_WITH_MISSING_FILE = "Creation with missing file";
const char *TEST_READER_CREATION_WITH_SIZE_MISMATCH = "Creation with file with size mismatch";
const char *TEST_READER_CREATION_WITH_TOO_FEW_ENTRIES = "Creation with file with too few entries";
const char *TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE = "Read header from newly opened file";
const char *TEST_READ_HEADER_TWICE = "Read header a second time";
const char *TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE = "Read index entry from newly opened file";
const char *TEST_READ_INDEX_FROM_FILE = "Read index entry from file";
const char *TEST_READ_SEVERAL_ENTRIES_FROM_FILE = "Read several index entries from file";
const char *TEST_READ_INDEX_FROM_END_OF_FILE = "Read index entry at end of file";

const size_t BASE_FILE_SIZE = sizeof(IndexEntryV1) + sizeof(IndexHeader);

auto createCharArray(int size) {
    boost::shared_ptr<char> arr(new char[size]);
    memset(arr.get(), 0, size);
    return arr;
}

path writeTestFile(TestResourcesAndFunctions *res, size_t size) {
    path idx = res->createEmptyFile(INDEX_FILENAME);

    boost::filesystem::ofstream out(idx);

    auto chars = createCharArray(size);

    out.write(chars.get(), size);
    out.flush();
    return idx;
}

SUITE (SUITE_INDEXREADER_TESTS) {
    TEST (TEST_READER_CREATION_WITH_EXISTING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_EXISTING_FILE);
        path idx = writeTestFile(&res, BASE_FILE_SIZE);
        auto ir = IndexReader::create(idx);
                CHECK(ir);
                CHECK_EQUAL(ir->getIndicesLeft(), 1);
    }

    TEST (TEST_READER_CREATION_WITH_UNREADABLE_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_UNREADABLE_FILE);
        path idx = res.createEmptyFile(INDEX_FILENAME);
        boost::filesystem::permissions(idx, owner_write | owner_exe);
        auto ir = IndexReader::create(idx);

                CHECK(!ir);
    }

    TEST (TEST_READER_CREATION_WITH_MISSING_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_MISSING_FILE);
        path idx = res.filePath(INDEX_FILENAME);
        auto ir = IndexReader::create(idx);

                CHECK(!ir);
    }

    TEST (TEST_READER_CREATION_WITH_SIZE_MISMATCH) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_SIZE_MISMATCH);
        path idx = writeTestFile(&res, BASE_FILE_SIZE + 3);
        auto ir = IndexReader::create(idx);

                CHECK(!ir);
    }

    TEST (TEST_READER_CREATION_WITH_TOO_FEW_ENTRIES) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READER_CREATION_WITH_TOO_FEW_ENTRIES);
        path idx = writeTestFile(&res, sizeof(IndexHeader));
        auto ir = IndexReader::create(idx);

                CHECK(!ir);
    }

    TEST (TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_HEADER_FROM_NEWLY_OPENED_FILE);
        path idx = writeTestFile(&res, BASE_FILE_SIZE);
        auto ir = IndexReader::create(idx);
        auto header = ir->readIndexHeader();

        ulong test[62] = {0};
        // The current test data is just a 0'ed file.
                CHECK(header);
                CHECK_EQUAL(header->indexWriterVersion, 0);
                CHECK_ARRAY_EQUAL(header->reserved, test, 62);
                CHECK_EQUAL(header->magicNumber, 0);
    }

    TEST (TEST_READ_HEADER_TWICE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_HEADER_TWICE);
        path idx = writeTestFile(&res, BASE_FILE_SIZE);
        auto ir = IndexReader::create(idx);
        auto header1 = ir->readIndexHeader();
        auto header2 = ir->readIndexHeader();
                CHECK(header1);
                CHECK(!header2);
    }

    TEST (TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_NEWLY_OPENED_FILE);
        path idx = writeTestFile(&res, BASE_FILE_SIZE);
        auto ir = IndexReader::create(idx);
        auto entry = ir->readIndexEntry();
                CHECK(!entry);
    }

    TEST (TEST_READ_INDEX_FROM_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_FILE);
        path idx = writeTestFile(&res, BASE_FILE_SIZE);
        auto ir = IndexReader::create(idx);
        auto header = ir->readIndexHeader();
        // The current test data is just a 0'ed file.
                CHECK(header);
        auto entry = ir->readIndexEntry();
                CHECK(entry);
                CHECK_EQUAL(entry->entryStartsWithLine, false);
                CHECK_EQUAL(entry->startingLineInEntry, 0);
                CHECK_EQUAL(entry->bits, 0);
                CHECK_EQUAL(entry->offset, 0);
                CHECK_EQUAL(entry->entryNumber, 0);
    }

    TEST (TEST_READ_SEVERAL_ENTRIES_FROM_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_SEVERAL_ENTRIES_FROM_FILE);
        path idx = writeTestFile(&res, BASE_FILE_SIZE + 4 * sizeof(IndexEntryV1));
        auto ir = IndexReader::create(idx);
        auto header = ir->readIndexHeader();
        // The current test data is just a 0'ed file.
                CHECK(header);
                CHECK(ir->readIndexEntry());
                CHECK(ir->readIndexEntry());
                CHECK(ir->readIndexEntry());
                CHECK(ir->readIndexEntry());
        auto entry = ir->readIndexEntry();
                CHECK(entry);
                CHECK_EQUAL(entry->entryStartsWithLine, false);
                CHECK_EQUAL(entry->startingLineInEntry, 0);
                CHECK_EQUAL(entry->bits, 0);
                CHECK_EQUAL(entry->offset, 0);
                CHECK_EQUAL(entry->entryNumber, 0);
    }

    TEST (TEST_READ_INDEX_FROM_END_OF_FILE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_END_OF_FILE);
        path idx = writeTestFile(&res, BASE_FILE_SIZE);
        auto ir = IndexReader::create(idx);
        auto header = ir->readIndexHeader();
        // The current test data is just a 0'ed file.
                CHECK(header);
                CHECK_EQUAL(1, ir->getIndicesLeft());
        auto entry1 = ir->readIndexEntry();
                CHECK_EQUAL(0, ir->getIndicesLeft());
        auto entry2 = ir->readIndexEntry();
                CHECK_EQUAL(0, ir->getIndicesLeft());
                CHECK(entry1);
                CHECK(!entry2); // Test below works? Why is the stream still good? It is too small...
    }

    TEST (TEST_COMBINED_READ_AND_WRITE) {
        TestResourcesAndFunctions res(SUITE_INDEXREADER_TESTS, TEST_READ_INDEX_FROM_END_OF_FILE);
        path idx = res.filePath(INDEX_FILENAME);
        auto header = boost::make_shared<IndexHeader>(1, sizeof(IndexEntryV1));
        auto entry0 = boost::make_shared<IndexEntryV1>(0, 10, 0, 0, 1);
        auto entry1 = boost::make_shared<IndexEntryV1>(0, 10, 0, 0, 0);
        auto entry2 = boost::make_shared<IndexEntryV1>(0, 10, 0, 0, 1);
        auto entry3 = boost::make_shared<IndexEntryV1>(0, 10, 0, 0, 0);

        auto writer = new IndexWriter(idx);
        bool open = writer->tryOpen();
                CHECK(open);
        writer->writeIndexHeader(header);
        writer->writeIndexEntry(entry0);
        writer->writeIndexEntry(entry1);
        writer->writeIndexEntry(entry2);
        writer->writeIndexEntry(entry3);

        delete writer;

        auto reader = IndexReader::create(idx);
                CHECK_EQUAL(reader->getIndicesLeft(), 4);

        auto readHeader = reader->readIndexHeader();
        auto readEntry0 = reader->readIndexEntry();
        auto readEntry1 = reader->readIndexEntry();
        auto readEntry2 = reader->readIndexEntry();
        auto readEntry3 = reader->readIndexEntry();

                CHECK(*header == *readHeader);
                CHECK(*entry0 == *readEntry0);
                CHECK(*entry1 == *readEntry1);
                CHECK(*entry2 == *readEntry2);
                CHECK(*entry3 == *readEntry3);
    }
}