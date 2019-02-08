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

const char* INDEXER_SUITE_TESTS = "IndexerTests";
const char* TEST_INDEXER_CREATION = "IndexerCreation";
const char* TEST_CREATE_INDEX = "testCreateIndex";

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
                CHECK(!indexer.getStoredHeader());

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
                CHECK_EQUAL(Indexer::INDEXER_VERSION, header->indexWriterVersion);
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

                CHECK(storedHeader);
                CHECK(storedHeader->indexWriterVersion == Indexer::INDEXER_VERSION);

                CHECK(storedEntries);
        if (storedEntries) // Without the if, other tests won't start! At least, we have an error message here and can go on.
                    CHECK (storedEntries->size() > 1);

                CHECK(storedLines);
        if (storedLines) // Without the if, other tests won't start!
                    CHECK (storedLines->size() == noOfEntriesInTestFastq);
    }
}