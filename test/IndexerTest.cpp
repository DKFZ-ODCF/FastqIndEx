/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/Runner.h"
#include "../src/Indexer.h"
#include "../src/IndexerRunner.h"
#include "../src/IndexReader.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>
#include <zlib.h>

const char *const INDEXER_SUITE_TESTS = "IndexerTests";
const char *const TEST_INDEXER_CREATION = "IndexerCreation";
const char *const TEST_CREATE_INDEX = "testCreateIndex";
const char *const TEST_INITIALIZE_ZSTREAM_WITH_ZEROES = "Initialize z_stream struct with some zeroes";
const char *const TEST_CREATE_INDEX_SMALL = "Test create index with small fastq test data.";
const char *const TEST_CREATE_INDEX_LARGE = "Test create index with more fastq test data.";

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

    TEST (TEST_INITIALIZE_ZSTREAM_WITH_ZEROES) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_INITIALIZE_ZSTREAM_WITH_ZEROES);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer indexer(fastq, index, true);
        z_stream zStream;
        bool ok = indexer.initializeZStream(&zStream);
                CHECK(ok);
                CHECK(zStream.zalloc != nullptr);
                CHECK(zStream.zfree != nullptr);    // Will be filled by inflateInit2
                CHECK(zStream.opaque == nullptr);   // Will be filled by inflateInit2
                CHECK(zStream.next_in == nullptr);
                CHECK(zStream.avail_in == 0);
    }

    // TEST ("readCompressedDataFromStream")  <-- How to write a test? Currently its covered in the larger tests.

    // TEST ("call createIndex() twice")

    TEST (TEST_CREATE_INDEX_SMALL) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_SMALL);

        path fastq = res.getResource(string("test.fastq.gz"));
        path index = res.filePath("test.fastq.gz.idx");

        Indexer indexer(fastq, index, true);
                CHECK(indexer.checkPremises());  // We need to make sure things are good. Also this opens the I-Writer.

        bool result = indexer.createIndex();

        auto storedHeader = indexer.getStoredHeader();
        auto storedEntries = indexer.getStoredEntries();
        auto storedLines = indexer.getStoredLines();

        int numberOfLinesInTestFASTQ = 4000;

                CHECK(result);
                CHECK(exists(index));
                CHECK(indexer.wasSuccessful());

                CHECK(storedHeader);
                CHECK(storedHeader->indexWriterVersion == Indexer::INDEXER_VERSION);

                CHECK(storedEntries.size() == 1);

                CHECK(storedLines.size() == numberOfLinesInTestFASTQ);
    }

    TEST (TEST_CREATE_INDEX_LARGE) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_LARGE);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer *indexer = new Indexer(fastq, index,
                                       true); // Tell the indexer to store entries. This is solely a debug feature but it
                CHECK(indexer->checkPremises());  // We need to make sure things are good. Also this opens the I-Writer.

        bool result = indexer->createIndex();

        auto storedHeader = indexer->getStoredHeader();
        auto storedEntries = indexer->getStoredEntries();
        auto storedLines = indexer->getStoredLines();

        int numberOfLinesInTestFASTQ = 160000;

                CHECK(result);
                CHECK(exists(index));
                CHECK(indexer->wasSuccessful());

                CHECK(storedHeader);
                CHECK(storedHeader->indexWriterVersion == Indexer::INDEXER_VERSION);

                CHECK(storedEntries.size() == 59); // How many exactly? Should be 59

                CHECK(storedLines.size() == numberOfLinesInTestFASTQ);

        // Now check the index file. We know, that there is one header and one entry.
        // Note, that the references (storedEntries/Lines...) above are not functional anymore! Don't use them.
        delete indexer;  // <-- Force flush and close!

        auto indexReader = IndexReader::create(index);
        auto header = indexReader->readIndexHeader();
        auto entry = indexReader->readIndexEntry();

                CHECK(header->indexWriterVersion == Indexer::INDEXER_VERSION);
                CHECK(entry->bits == 7);
                CHECK(entry->offsetOfFirstValidLine == 0);
                CHECK(entry->relativeBlockOffsetInRawFile == 10);
                CHECK(entry->startingLineInEntry == 0);

        indexReader.reset();
    }

}