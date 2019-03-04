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

    TEST (TEST_CALCULATE_BLOCK_INTERVAL) {
        const ulong GB = 1024 * 1024 * 1024;
                CHECK_EQUAL(16, Indexer::calculateIndexBlockInterval(1 * GB));
                CHECK_EQUAL(32, Indexer::calculateIndexBlockInterval(2 * GB));
                CHECK_EQUAL(64, Indexer::calculateIndexBlockInterval(4 * GB));
                CHECK_EQUAL(128, Indexer::calculateIndexBlockInterval(8 * GB));
                CHECK_EQUAL(512, Indexer::calculateIndexBlockInterval(20 * GB));
                CHECK_EQUAL(512, Indexer::calculateIndexBlockInterval(30 * GB));
                CHECK_EQUAL(1024, Indexer::calculateIndexBlockInterval(40 * GB));
                CHECK_EQUAL(2048, Indexer::calculateIndexBlockInterval(80 * GB));
                CHECK_EQUAL(2048, Indexer::calculateIndexBlockInterval(120 * GB));
                CHECK_EQUAL(4096, Indexer::calculateIndexBlockInterval(160 * GB));
                CHECK_EQUAL(4096, Indexer::calculateIndexBlockInterval(200 * GB));
                CHECK_EQUAL(8192, Indexer::calculateIndexBlockInterval(300 * GB));
        //Maximum value
                CHECK_EQUAL(8192, Indexer::calculateIndexBlockInterval(430 * GB));
                CHECK_EQUAL(8192, Indexer::calculateIndexBlockInterval(1630 * GB));
    }

    TEST (TEST_INDEXER_CREATION) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_INDEXER_CREATION);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer indexer(fastq, index, -1);

                CHECK_EQUAL(fastq, indexer.getFastq());
                CHECK_EQUAL(index, indexer.getIndex());
                CHECK_EQUAL(false, indexer.isDebuggingEnabled());
                CHECK_EQUAL(false, indexer.wasSuccessful());
                CHECK_EQUAL(0, indexer.getFoundEntries());
                CHECK(!indexer.getStoredHeader());

        indexer = Indexer(fastq, index, -1, true);
                CHECK_EQUAL(true, indexer.isDebuggingEnabled());
    }

    TEST (TEST_CREATE_HEADER) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer indexer(fastq, index, -1,
                        true); // Tell the indexer to store entries. This is solely a debug feature but it
        boost::shared_ptr<IndexHeader> header = indexer.createHeader();
                CHECK(header.get());
                CHECK_EQUAL(Indexer::INDEXER_VERSION, header->indexWriterVersion);
    }

    TEST (TEST_INITIALIZE_ZSTREAM_WITH_ZEROES) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_INITIALIZE_ZSTREAM_WITH_ZEROES);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        Indexer indexer(fastq, index, -1, true);
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

        Indexer *indexer = new Indexer(fastq, index, -1, true);
                CHECK(indexer->checkPremises());  // We need to make sure things are good. Also this opens the I-Writer.

        bool result = indexer->createIndex();

        auto storedHeader = indexer->getStoredHeader();
        auto storedEntries = indexer->getStoredEntries();
        auto storedLines = indexer->getStoredLines();

        int numberOfLinesInTestFASTQ = 4000;

                CHECK(result);
                CHECK(exists(index));
                CHECK(indexer->wasSuccessful());

                CHECK(storedHeader);
                CHECK_EQUAL(Indexer::INDEXER_VERSION, storedHeader->indexWriterVersion);

                CHECK_EQUAL(1, storedEntries.size());
                CHECK_EQUAL(numberOfLinesInTestFASTQ, storedLines.size());

        // Why is this a pointer? Just to get access to the file on the command line. It is written if the
        // Indexer is delete OR enough data was written. If we do not have the pointer, the file gets written after the
        // test is finished.
        delete indexer;
                CHECK(exists(index));
    }

    TEST (TEST_CREATE_INDEX_LARGE) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_LARGE);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

        uint blockSize = 4;

        Indexer *indexer = new Indexer(
                fastq,
                index,
                blockSize,
                true
        ); // Tell the indexer to store entries. This is solely a debug feature but it
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
                CHECK_EQUAL(Indexer::INDEXER_VERSION, storedHeader->indexWriterVersion);

        // How many exactly? Should be 59 for interval 1 and 15 for interval 4
                CHECK_EQUAL(15, storedEntries.size());
                CHECK_EQUAL(numberOfLinesInTestFASTQ, storedLines.size());


        // CHECK for these values, they are taken from the original program.
                CHECK_EQUAL(0, storedEntries[0]->bits);
                CHECK_EQUAL(10, storedEntries[0]->blockOffsetInRawFile);
                CHECK_EQUAL(6, storedEntries[1]->bits);
                CHECK_EQUAL(219567, storedEntries[1]->blockOffsetInRawFile);
                CHECK_EQUAL(0, storedEntries[2]->bits);
                CHECK_EQUAL(438941, storedEntries[2]->blockOffsetInRawFile);
                CHECK_EQUAL(6, storedEntries[3]->bits);
                CHECK_EQUAL(658928, storedEntries[3]->blockOffsetInRawFile);

        // The following "table" contains index data originally created with zran.c to test our indexer algorithm
        // Note, that this test works with an block interval of 4, so only every 4th line is relevant. All other lines
        // are indented.
        //  [ bits, offset ] : [ 0 , 10 ]
        //    [ bits, offset ] : [ 7 , 54866 ]
        //    [ bits, offset ] : [ 3 , 108492 ]
        //    [ bits, offset ] : [ 7 , 163881 ]
        //  [ bits, offset ] : [ 6 , 219567 ]
        //    [ bits, offset ] : [ 5 , 275159 ]
        //    [ bits, offset ] : [ 3 , 330901 ]
        //    [ bits, offset ] : [ 5 , 385834 ]
        //  [ bits, offset ] : [ 0 , 438941 ]
        //    [ bits, offset ] : [ 2 , 493345 ]
        //    [ bits, offset ] : [ 0 , 548300 ]
        //    [ bits, offset ] : [ 6 , 603576 ]
        //  [ bits, offset ] : [ 6 , 658928 ]
        //    [ bits, offset ] : [ 4 , 714162 ]
        //    [ bits, offset ] : [ 7 , 768171 ]
        //    [ bits, offset ] : [ 4 , 821077 ]
        //  [ bits, offset ] : [ 2 , 876315 ]
        //    [ bits, offset ] : [ 1 , 931253 ]
        //    [ bits, offset ] : [ 5 , 986786 ]
        //    [ bits, offset ] : [ 1 , 1042386 ]
        //  [ bits, offset ] : [ 6 , 1097810 ]
        //    [ bits, offset ] : [ 0 , 1150682 ]
        //    [ bits, offset ] : [ 7 , 1205489 ]
        //    [ bits, offset ] : [ 5 , 1260492 ]
        //  [ bits, offset ] : [ 3 , 1315485 ]
        //    [ bits, offset ] : [ 1 , 1370562 ]
        //    [ bits, offset ] : [ 7 , 1425292 ]
        //    [ bits, offset ] : [ 3 , 1479030 ]
        //  [ bits, offset ] : [ 7 , 1532498 ]
        //    [ bits, offset ] : [ 2 , 1588182 ]
        //    [ bits, offset ] : [ 0 , 1643214 ]
        //    [ bits, offset ] : [ 6 , 1698756 ]
        //  [ bits, offset ] : [ 0 , 1754593 ]
        //    [ bits, offset ] : [ 6 , 1809191 ]
        //    [ bits, offset ] : [ 5 , 1862340 ]
        //    [ bits, offset ] : [ 1 , 1917303 ]
        //  [ bits, offset ] : [ 2 , 1972535 ]
        //    [ bits, offset ] : [ 5 , 2027836 ]
        //    [ bits, offset ] : [ 1 , 2083220 ]
        //    [ bits, offset ] : [ 1 , 2137888 ]
        //  [ bits, offset ] : [ 7 , 2191118 ]
        //    [ bits, offset ] : [ 1 , 2197466 ]
        //    [ bits, offset ] : [ 0 , 2252018 ]
        //    [ bits, offset ] : [ 3 , 2307589 ]
        //  [ bits, offset ] : [ 7 , 2363170 ]
        //    [ bits, offset ] : [ 3 , 2418775 ]
        //    [ bits, offset ] : [ 0 , 2474206 ]
        //    [ bits, offset ] : [ 5 , 2528051 ]
        //  [ bits, offset ] : [ 6 , 2581647 ]
        //    [ bits, offset ] : [ 0 , 2637003 ]
        //    [ bits, offset ] : [ 5 , 2692023 ]

        /**
 * Here we're going to prepare the test data for our line-by-line test. But only, if the preceding tests were
 * successful.
 */
        path extractedFastq = res.filePath("test.fastq");
        string command = (
                string("gunzip -c \"") + fastq.string() + "\"" +
                " > \"" + extractedFastq.string() + "\""
        );

        int success = std::system(command.c_str());
                CHECK_EQUAL(0, success);

        boost::filesystem::ifstream strm(extractedFastq);
        vector<string> decompressedSourceContent;
        string line;
        while (std::getline(strm, line)) {
            decompressedSourceContent.emplace_back(line);
        }
                CHECK_EQUAL(160000, decompressedSourceContent.size());

        for(int i=0;i<160000;i++) {
            auto equal = decompressedSourceContent[i] == storedLines[i];
            if(!equal)
                CHECK_EQUAL(true, equal);
        }

        // Now check the index file in a very simple way (extractor tests come later). We know, that there is one header
        // and several entries.
        // Note, that the references (storedEntries/Lines...) above are not functional anymore! Don't use them.
        delete indexer;  // <-- Force flush and close file stream!

        // Read back and check contents.
        IndexReader *ir = new IndexReader(index);
                CHECK(ir->tryOpenAndReadHeader());

        for (int i = 0; i < storedEntries.size(); i++) {
            auto entry = ir->readIndexEntryV1();
            auto l = *storedEntries[i];
            auto r = *entry;
                    CHECK(l == r);
        }

        delete ir;
    }
}