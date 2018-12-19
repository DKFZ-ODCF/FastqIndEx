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

const string INDEXER_SUITE_TESTS = "IndexerTests";
const string TEST_INDEXER_CREATION = "IndexerCreation";
const string TEST_CREATE_INDEX = "testCreateIndex";

SUITE (INDEXER_SUITE_TESTS) {
    TEST (TEST_INDEXER_CREATION) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_INDEXER_CREATION);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer indexer(fastq, index);

                CHECK_EQUAL(fastq, indexer.getFastq());
                CHECK_EQUAL(index, indexer.getIndex());
                CHECK_EQUAL(false, indexer.isDebuggingEnabled());
                CHECK_EQUAL(false, indexer.wasSuccessful());
                CHECK_EQUAL(0, indexer.getFoundEntries());
                CHECK(!indexer.getStoredHeader().get());

        indexer = Indexer(fastq, index, true);
                CHECK_EQUAL(true, indexer.isDebuggingEnabled());
    }

    TEST (TEST_CREATE_HEADER) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer indexer(fastq, index, true); // Tell the indexer to store entries. This is solely a debug feature but it
        boost::shared_ptr<IndexHeader> header = indexer.createHeader();
        CHECK(header.get());
        CHECK_EQUAL(Indexer::INDEXER_VERSION, header->binary_version);
    }

    TEST (TEST_CREATE_INDEX) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer indexer(fastq, index, true); // Tell the indexer to store entries. This is solely a debug feature but it

        bool result = indexer.createIndex();

        auto storedHeader = indexer.getStoredHeader();
        auto storedEntries = indexer.getStoredEntries();
        auto storedLines = indexer.getStoredLines();

        int noOfEntriesInTestFastq = 160000;

                CHECK_EQUAL(true, result);
                CHECK(exists(index));
                CHECK(indexer.wasSuccessful());
                CHECK_EQUAL(noOfEntriesInTestFastq, indexer.getFoundEntries());

                CHECK(storedHeader.get());
                CHECK(storedHeader.get()->binary_version == Indexer::INDEXER_VERSION);

                CHECK(storedEntries.get());
                CHECK(storedEntries->size() > 1);

                CHECK(storedLines.get());
                CHECK(storedLines->size() == noOfEntriesInTestFastq);
    }
}