/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/StringHelper.h"
#include "process/extract/IndexReader.h"
#include "process/index/Indexer.h"
#include "process/io/PathSource.h"
#include "process/io/PathSink.h"
#include "process/io/StreamSource.h"
#include "runners/IndexerRunner.h"
#include "TestResourcesAndFunctions.h"
#include <fstream>
#include <memory>
#include <UnitTest++/UnitTest++.h>
#include <zlib.h>

const char *const INDEXER_SUITE_TESTS = "IndexerTests";
const char *const TEST_INDEXER_CREATION = "IndexerCreation";
const char *const TEST_CORRECT_BLOCK_LINE_COUNTING = "Test the correct counting of lines in decompressed blocks.";
const char *const TEST_CREATE_INDEX = "testCreateIndex";
const char *const TEST_CREATE_INDEX_SMALL = "Test create index with small fastq test data.";
const char *const TEST_CREATE_INDEX_W_STREAMED_DATA = "Test create index with streamed concatenated data";
const char *const TEST_CREATE_INDEX_LARGE = "Test create index with more fastq test data.";
const char *const TEST_CREATE_INDEX_CONCAT = "Test create index with the small fastq concatenated two times.";
const char *const TEST_CREATE_INDEX_CONCAT_SINGLEBLOCKS = "Test create index with several concatenated FASTQ with single compressed blocks.";

SUITE (INDEXER_SUITE_TESTS) {

    TEST (TEST_INDEXER_CREATION) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_INDEXER_CREATION);

        path fastq = res.getResource(string(TEST_FASTQ_LARGE));
        path index = res.filePath("test2.fastq.gz.fqi");

        auto *indexer = new Indexer(
                make_shared<PathSource>(fastq),
                make_shared<PathSink>(index),
                BlockDistanceStorageStrategy::getDefault()
        );

                CHECK_EQUAL(fastq, dynamic_pointer_cast<PathSource>(indexer->getFastq())->getPath());
                CHECK_EQUAL(index.string(), indexer->getOutputIndexFile()->toString());
                CHECK_EQUAL(false, indexer->isDebuggingEnabled());
                CHECK_EQUAL(false, indexer->wasSuccessful());
                CHECK_EQUAL(0, indexer->getFoundEntries());
                CHECK(!indexer->getStoredHeader());

        delete indexer;
        indexer = new Indexer(make_shared<PathSource>(fastq),
                              make_shared<PathSink>(index),
                              BlockDistanceStorageStrategy::getDefault(),
                              true
        );
                CHECK_EQUAL(true, indexer->isDebuggingEnabled());
        delete indexer;
    }

    TEST (TEST_CREATE_HEADER) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX);

        path fastq = res.getResource(TEST_FASTQ_LARGE);
        path index = res.filePath("test2.fastq.gz.fqi");

        Indexer indexer(make_shared<PathSource>(fastq),
                        make_shared<PathSink>(index),
                        BlockDistanceStorageStrategy::getDefault(),
                        true); // Tell the indexer to store entries. This is solely a debug feature but it
        shared_ptr<IndexHeader> header = indexer.createHeader();
                CHECK(header.get());
                CHECK_EQUAL(Indexer::INDEXER_VERSION, header->indexWriterVersion);
    }

    TEST (TEST_CORRECT_BLOCK_LINE_COUNTING) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CORRECT_BLOCK_LINE_COUNTING);
        vector<string> _blockData = TestResourcesAndFunctions::getTestVectorWithSimulatedBlockData();

        // The vector contains IndexEntries with some expected values: line offset, starting line
        // This is more to keep things clear and easily readable.
        vector<IndexEntry> expectedIndexEntries;
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 0));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 3));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 3, 0, 6));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 9));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 9));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 3, 0, 9));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 12));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 2, 0, 12));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 15));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 15));
        expectedIndexEntries.emplace_back(IndexEntry(0, 0, 0, 0, 18));

        u_int32_t expectedNumberOfLinesInBlock[] = {3, 3, 3, 0, 0, 3, 0, 3, 0, 3, 3};

        // Files are not actually used.
        path fastq = res.getResource(TEST_FASTQ_LARGE);
        path index = res.filePath("test2.fastq.gz.fqi");
        Indexer indexer(make_shared<PathSource>(fastq),
                        make_shared<PathSink>(index),
                        BlockDistanceStorageStrategy::getDefault(),
                        true
        );

        bool lastBlockEndedWithNewline = true;

        for (u_int64_t i = 0; i < _blockData.size(); i++) {
            auto blockData = _blockData[i];
            auto split = StringHelper::splitStr(blockData);
            auto expectedNumberOfLines = expectedNumberOfLinesInBlock[i];
            auto expectedFirstLineOffset = expectedIndexEntries[i].offsetToNextLineStart;
            auto expectedStartingLine = expectedIndexEntries[i].startingLineInEntry;

            int64_t off = 0;
            bool currentBlockEndedWithNewline;
            u_int32_t numberOfLinesInBlock;
            auto entry = indexer.createIndexEntryFromBlockData(blockData, split, off, lastBlockEndedWithNewline,
                                                               &currentBlockEndedWithNewline, &numberOfLinesInBlock);
                    CHECK(entry->blockOffsetInRawFile == 0);
                    CHECK(entry->offsetToNextLineStart == expectedFirstLineOffset);
                    CHECK(entry->startingLineInEntry == expectedStartingLine);
                    CHECK(numberOfLinesInBlock == expectedNumberOfLines);

            lastBlockEndedWithNewline = currentBlockEndedWithNewline;
        }
    }

    TEST (TEST_CREATE_INDEX_CONCAT_SINGLEBLOCKS) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_CONCAT_SINGLEBLOCKS);
        path fastq = res.getResource("test_singlecompressedblocks.fastq.gz");
        path index = res.filePath("test.fastq.gz.fqi");
        path extractedFastq = res.filePath(TEST_FASTQ_SMALL);
        auto *indexer = new Indexer(make_shared<PathSource>(fastq), make_shared<PathSink>(index),
                                    BlockDistanceStorageStrategy::getDefault(),
                                    true,
                                    false,
                                    false,
                                    true
        );
                CHECK(indexer->fulfillsPremises());  // We need to make sure things are good. Also this opens the I-Writer.

        bool result = indexer->createIndex();

        auto storedHeader = indexer->getStoredHeader();
        auto storedEntries = indexer->getStoredEntries();
        auto storedLines = indexer->getStoredLines();

        int numberOfLinesInTestFASTQ = 800;
        int storedLineCount = storedLines.size();

                CHECK(result);
                CHECK(exists(index));
                CHECK(indexer->wasSuccessful());
                CHECK(indexer->getNumberOfConcatenatedFiles() == 8);

                CHECK(storedHeader);
                CHECK(Indexer::INDEXER_VERSION == storedHeader->indexWriterVersion);

                CHECK(1 == storedEntries.size());
                CHECK(numberOfLinesInTestFASTQ == storedLineCount);

        result = TestResourcesAndFunctions::extractGZFile(fastq, extractedFastq);
                CHECK_EQUAL(true, result);

        vector<string> decompressedSourceContent = TestResourcesAndFunctions::readLinesOfFile(extractedFastq);

                CHECK(TestResourcesAndFunctions::compareVectorContent(storedLines, decompressedSourceContent));

        delete indexer;
    }

    TEST (TEST_CREATE_INDEX_CONCAT) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_SMALL);

        path fastq = res.getResource(TEST_FASTQ_SMALL);
        path concat = res.filePath("test_concat.fastq.gz");
        path index = res.filePath("test_concat.fastq.gz.fqi");
        path extractedFastq = res.filePath(TEST_FASTQ_SMALL);


        int appendCount = 4;
        bool result = TestResourcesAndFunctions::createConcatenatedFile(fastq, concat, appendCount);
                CHECK_EQUAL(true, result);
                CHECK(4 * file_size(fastq) == file_size(concat));

        auto *indexer = new Indexer(make_shared<PathSource>(concat),
                                    make_shared<PathSink>(index),
                                    BlockDistanceStorageStrategy::getDefault(),
                                    true,
                                    false,
                                    false,
                                    true
        );
                CHECK(indexer->fulfillsPremises());  // We need to make sure things are good. Also this opens the I-Writer.

        result = indexer->createIndex();

        auto storedHeader = indexer->getStoredHeader();
        auto storedEntries = indexer->getStoredEntries();
        auto storedLines = indexer->getStoredLines();

        int numberOfLinesInTestFASTQ = appendCount * 4000;
        int storedLineCount = storedLines.size();
                CHECK(result);
                CHECK(exists(index));
                CHECK(indexer->wasSuccessful());

                CHECK(storedHeader);
                CHECK_EQUAL(Indexer::INDEXER_VERSION, storedHeader->indexWriterVersion);

                CHECK_EQUAL(1U, storedEntries.size());
                CHECK(numberOfLinesInTestFASTQ == storedLineCount);

        result = TestResourcesAndFunctions::extractGZFile(concat, extractedFastq);
                CHECK_EQUAL(true, result);

        vector<string> decompressedSourceContent = TestResourcesAndFunctions::readLinesOfFile(extractedFastq);

                CHECK(TestResourcesAndFunctions::compareVectorContent(storedLines, decompressedSourceContent));

        // Why is this a pointer? Just to get access to the file on the command line. It is written if the
        // Indexer is delete OR enough data was written. If we do not have the pointer, the file gets written after the
        // test is finished.
        delete indexer;
                CHECK(exists(index));
    }

    TEST (TEST_CREATE_INDEX_SMALL) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_SMALL);

        path fastq = res.getResource(string(TEST_FASTQ_SMALL));
        path index = res.filePath("test.fastq.gz.fqi");

        auto indexer = new Indexer(make_shared<PathSource>(fastq), make_shared<PathSink>(index),
                                   BlockDistanceStorageStrategy::getDefault(), true, false, false,
                                   true);
                CHECK(indexer->fulfillsPremises());  // We need to make sure things are good. Also this opens the I-Writer.

        bool result = indexer->createIndex();

        auto storedHeader = indexer->getStoredHeader();
        auto storedEntries = indexer->getStoredEntries();
        auto storedLines = indexer->getStoredLines();

        uint64_t numberOfLinesInTestFASTQ = 4000;

                CHECK(result);
                CHECK(exists(index));
                CHECK(indexer->wasSuccessful());

                CHECK(storedHeader);
                CHECK_EQUAL(Indexer::INDEXER_VERSION, storedHeader->indexWriterVersion);

                CHECK_EQUAL(1U, storedEntries.size());
                CHECK_EQUAL(numberOfLinesInTestFASTQ, storedLines.size());

        // Why is this a pointer? Just to get access to the file on the command line. It is written if the
        // Indexer is delete OR enough data was written. If we do not have the pointer, the file gets written after the
        // test is finished.
        delete indexer;
                CHECK(exists(index));
    }

    TEST (TEST_CREATE_INDEX_W_STREAMED_DATA) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_SMALL);
        path fastq = res.getResource(TEST_FASTQ_SMALL);
        path concat = res.filePath("test_concat.fastq.gz");
        path index = res.filePath("test_concat.fastq.gz.fqi");
        path extractedFastq = res.filePath(TEST_FASTQ_SMALL);


        int appendCount = 4;
        bool result = TestResourcesAndFunctions::createConcatenatedFile(fastq, concat, appendCount);
                CHECK(result);
                CHECK(4 * file_size(fastq) == file_size(concat));

        ifstream fastqStream(concat.string());
        auto indexer = new Indexer(
                make_shared<StreamSource>(&fastqStream),
                make_shared<PathSink>(index),
                BlockDistanceStorageStrategy::from(1),
                true,
                false,
                false,
                true
        );

        indexer->createIndex();
        auto storedHeader = indexer->getStoredHeader();
        auto storedEntries = indexer->getStoredEntries();
        auto storedLines = indexer->getStoredLines();

                CHECK(indexer->wasSuccessful());
                CHECK(8 == storedEntries.size());

        delete indexer;
                CHECK(exists(index));
    }

    TEST (testIndexerIntegrationTestWPipedSmallDataset) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, "testIndexerIntegrationTestWPipedSmallDataset");
        path fastq = res.getResource(string(TEST_FASTQ_LARGE));
        path index = res.filePath("test2.fastq.gz.fqi");
        ifstream fqStream(fastq);
        auto runner = new IndexerRunner(
                StreamSource::from(&fqStream),
                PathSink::from(index),
                BlockDistanceStorageStrategy::getDefault(),
                false, false, false, true);
                CHECK(runner->run() == 0);
        delete runner;
                CHECK(file_size(index) > 0);
    }

    TEST (TEST_CREATE_INDEX_LARGE) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_INDEX_LARGE);

        path fastq = res.getResource(string(TEST_FASTQ_LARGE));
        path index = res.filePath("test2.fastq.gz.fqi");

        uint blockSize = 4;

        auto *indexer = new Indexer(
                make_shared<PathSource>(fastq),
                make_shared<PathSink>(index),
                BlockDistanceStorageStrategy::from(blockSize),
                true, false, false, true
        ); // Tell the indexer to store entries. This is solely a debug feature but it
                CHECK(indexer->fulfillsPremises());  // We need to make sure things are good. Also this opens the I-Writer.

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
                CHECK_EQUAL(15U, storedEntries.size());
                CHECK_EQUAL(numberOfLinesInTestFASTQ, storedLines.size());


        // CHECK for these values, they are taken from the original program.
                CHECK_EQUAL(0U, storedEntries[0]->bits);
                CHECK_EQUAL(10U, storedEntries[0]->blockOffsetInRawFile);
                CHECK_EQUAL(6U, storedEntries[1]->bits);
                CHECK_EQUAL(219567U, storedEntries[1]->blockOffsetInRawFile);
                CHECK_EQUAL(0U, storedEntries[2]->bits);
                CHECK_EQUAL(438941U, storedEntries[2]->blockOffsetInRawFile);
                CHECK_EQUAL(6U, storedEntries[3]->bits);
                CHECK_EQUAL(658928U, storedEntries[3]->blockOffsetInRawFile);

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

        // Here we're going to prepare the test data for our line-by-line test. But only, if the preceding tests were
        // successful.
        path extractedFastq = res.filePath("test.fastq");

        result = TestResourcesAndFunctions::extractGZFile(fastq, extractedFastq);
                CHECK_EQUAL(true, result);

        vector<string> decompressedSourceContent = TestResourcesAndFunctions::readLinesOfFile(extractedFastq);
                CHECK_EQUAL(160000, decompressedSourceContent.size());

                CHECK(TestResourcesAndFunctions::compareVectorContent(storedLines, decompressedSourceContent));

        // Now check the index file in a very simple way (extractor tests come later). We know, that there is one header
        // and several entries.
        // Note, that the references (storedEntries/Lines...) above are not functional anymore! Don't use them.
        delete indexer;  // <-- Force flush and close file stream!

        // Read back and check contents.
        auto *ir = new IndexReader(make_shared<PathSource>(index));
                CHECK(ir->tryOpenAndReadHeader());

        for (auto &storedEntry : storedEntries) {
            auto entry = ir->readIndexEntryV1();
            auto l = *storedEntry;
            auto r = *entry;
                    CHECK(l == r);
        }

        delete ir;
    }
}