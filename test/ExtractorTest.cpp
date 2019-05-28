/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/Runner.h"
#include "../src/Extractor.h"
#include "../src/ExtractorRunner.h"
#include "../src/Indexer.h"
#include "../src/PathInputSource.h"
#include "TestResourcesAndFunctions.h"
#include <fstream>
#include <iostream>
#include <UnitTest++/UnitTest++.h>
#include <zlib.h>

const char *const INDEXER_SUITE_TESTS = "IndexerTests";
const char *const TEST_EXTRACTOR_CREATION = "Extractor creation";
const char *const TEST_CREATE_EXTRACTOR_AND_EXTRACT_SMALL_TO_COUT = "Combined test for index creation and extraction with the small dataset, extracts to cout.";
const char *const TEST_CREATE_EXTRACTOR_AND_EXTRACT_LARGE_TO_COUT = "Combined test for index creation and extraction with the larger dataset, extracts to cout.";
const char *const TEST_CREATE_EXTRACTOR_AND_EXTRACT_CONCAT_TO_COUT = "Combined test for index creation and extraction with the concatenated dataset, extracts to cout.";
const char *const TEST_PROCESS_DECOMPRESSED_DATA = "Test processDecompressedData() with some test data files (analogous to IndexerTest::TEST_CORRECT_BLOCK_LINE_COUNTING.)";
const char *const TEST_EXTRACTOR_CHECKPREM_OVERWRITE_EXISTING = "Test fail on exsiting file with disabled overwrite.";
const char *const TEST_EXTRACTOR_CHECKPREM_MISSING_NOTWRITABLE = "Test fail on non-writable result file with overwrite enabled.";
const char *const TEST_EXTRACTOR_CHECKPREM_MISSING_PARENTNOTWRITABLE = "Test fail on non-writable output folder.";
const char *const TEST_EXTRACTOR_EXTRACT_WITH_OUTFILE = "Text extract to an output file.";
const char *const TEST_EXTRACTOR_EXTRACT_WITH_EXISTINGOUTFILE = "Text extract to an output file which already exists.";


void runRangedExtractionTest(const path &fastq,
                             const path &index,
                             const vector<string> &decompressedSourceContent,
                             const u_int64_t firstLine,
                             const u_int64_t lineCount,
                             const u_int64_t expectedLineCount) {
    auto *extractor = new Extractor(make_shared<PathInputSource>(fastq), index, "-", false, firstLine, lineCount, 4,
                                    true);
    bool ok = extractor->extract();
            CHECK(ok);
    if (!ok) {
        for (int i = 0; i < extractor->getErrorMessages().size(); i++) {
            cout << extractor->getErrorMessages()[i] << "\n";
        }
    }

    vector<string> lines = extractor->getStoredLines();
    delete extractor;
    if (!ok)
        return;

            CHECK(expectedLineCount == lines.size());

            CHECK(TestResourcesAndFunctions::compareVectorContent(decompressedSourceContent, lines, firstLine));
}

bool initializeComplexTest(const path &fastq,
                           const path &index,
                           const path &extractedFastq,
                           const int blockInterval,
                           u_int64_t decompressedLineCount,
                           vector<string> *decompressedSourceContent) {
/**
     * Create the index fresh from the FASTQ, so we do not need to store the index file in our resources, also makes
     * debugging the indexer easier.
     */
    auto *indexer = new Indexer(make_shared<PathInputSource>(fastq), index, blockInterval, true, false, false, true);
            CHECK(indexer->checkPremises());
    indexer->enableWriteOutOfDecompressedBlocksAndStatistics(index.parent_path());
    indexer->createIndex();
    bool success = indexer->wasSuccessful();
            CHECK(success);
    delete indexer;
    if (!success) {
        return false;
    }

    /**
     * Check premises first. As we are going to run more than one test, we'll delete this instantly and use new
     * instances everytime.
     */
    auto *extractor = new Extractor(make_shared<PathInputSource>(fastq), index, "-", false, 0, 10, 4, true);
    bool premisesMet = extractor->checkPremises();
            CHECK(premisesMet);
    delete extractor;
    if (!premisesMet)
        return false;

    /**
     * Here we're going to prepare the test data for our line-by-line test. But only, if the preceding tests were
     * successful.
     */
    bool result = TestResourcesAndFunctions::extractGZFile(fastq, extractedFastq);
            CHECK_EQUAL(true, result);

    ifstream strm(extractedFastq);
    string line;
    while (std::getline(strm, line)) {
        decompressedSourceContent->emplace_back(line);
    }
            CHECK_EQUAL(decompressedLineCount, decompressedSourceContent->size());

    return success;
}

SUITE (INDEXER_SUITE_TESTS) {

    TEST (TEST_EXTRACTOR_CREATION) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_EXTRACTOR_CREATION);

        path fastq = res.getResource(TEST_FASTQ_SMALL);
        path index = res.getResource(TEST_INDEX_SMALL);

        auto *extractor = new Extractor(make_shared<PathInputSource>(fastq), index, "-", false, 0, 10, 4, true);
                CHECK(extractor->checkPremises());
        delete extractor;
    }
//
//    TEST (TEST_EXTRACTOR_CHECKPREM_OVERWRITE_EXISTING) {
//                CHECK(false);
//    }
//
//    TEST (TEST_EXTRACTOR_CHECKPREM_MISSING_NOTWRITABLE) {
//                CHECK(false);
//    }
//
//    TEST (TEST_EXTRACTOR_CHECKPREM_MISSING_PARENTNOTWRITABLE) {
//                CHECK(false);
//    }
//
//    TEST (TEST_EXTRACTOR_EXTRACT_WITH_OUTFILE) {
//        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_EXTRACTOR_EXTRACT_WITH_OUTFILE);
//
//        res.getResource(TEST_FASTQ_SMALL);
//
//                CHECK(false);
//    }
//
//    TEST (TEST_EXTRACTOR_EXTRACT_WITH_EXISTINGOUTFILE) {
//        // Check, that the output file size matches!
//                CHECK(false);
//    }

    TEST (TEST_PROCESS_DECOMPRESSED_DATA) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_PROCESS_DECOMPRESSED_DATA);
        vector<string> _blockData;
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_0_complete"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_1_endopen"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_2_startendopen"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_5_empty"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_5_empty"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_3_startendopen"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_3.1_startendopen_nonewlines"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_4_startopen"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_5_empty"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_6_newlines"));
        _blockData.emplace_back(res.readResourceFile("blockAndLineCalculations/block_7_finalwonewlines"));

        path fastq = res.getResource(TEST_FASTQ_LARGE);
        path index = res.filePath("test2.fastq.gz.fqi");

        vector<shared_ptr<IndexEntryV1>> indexEntries;

        bool lastBlockEndedWithNewline = true;
        Indexer indexer(make_shared<PathInputSource>(fastq), index, 1, true);
        for (auto bd : _blockData) {
            auto split = ZLibBasedFASTQProcessorBaseClass::splitStr(bd);
            bool currentBlockEndedWithNewline;
            u_int32_t numberOfLinesInBlock;
            u_int64_t offset = 0;
            auto entry = indexer.createIndexEntryFromBlockData(bd, split, offset, lastBlockEndedWithNewline,
                                                               &currentBlockEndedWithNewline,
                                                               &numberOfLinesInBlock);
            indexEntries.emplace_back(entry);
            lastBlockEndedWithNewline = currentBlockEndedWithNewline;
        }

        uint startingLines[]{0, 4, 8, 12, 16, 18};
        int indexEntryID[]{0, 1, 2, 7, 9, 7};
        int startingBlockIDs[]{0, 1, 2, 7, 8, 7};
        int expectedLines[]{4, 4, 4, 4, 4, 2};

        // The Extractor works a bit differently than the Indexer. In the Indexer, we decompress whole blocks, whereas
        // in the Extractor, we decompress chunk-wise (technical reasons, the flush mode Z_BLOCK does not work). One
        // problem here is, that the skip can be bigger than the decompressed chunk of data and this lead to problems
        // in the past. To test this, we start with a "lower" index entry and add a skip factor for the LAST test!

        // Please also note, that the last line of the test dataset has NO newline, which can also occur in some source
        // files. This line will not be found by the tested method here but will be output with a WARNING at the end of
        // the extraction process.

        for (int i = 0; i < 6; i++) {
            auto startingLine = startingLines[i];
            auto lineCount = 4;
            auto indexEntry = indexEntries[indexEntryID[i]];
            Extractor extractor(make_shared<PathInputSource>(fastq), index, path("-"), false, startingLine, lineCount,
                                4, true);
            extractor.setSkip(startingLine - indexEntry->startingLineInEntry);
            ostringstream outStream;

            for (int j = startingBlockIDs[i]; j < _blockData.size(); j++) {
                if (extractor.processDecompressedData(&outStream, _blockData[j], indexEntry->toIndexEntry()))
                    extractor.setFirstPass(false);
            }
            auto split = ZLibBasedFASTQProcessorBaseClass::splitStr(outStream.str());
                    CHECK(extractor.getStoredLines().size() == expectedLines[i]);
        }
    }

    TEST (TEST_CREATE_EXTRACTOR_AND_EXTRACT_SMALL_TO_COUT) {
        // We won't test the indexing here as it is already tested in TEST_CREATE_INDEX_SMALL
        // What we do is:
        // - Create the index
        // - We decompress the fastq file with gunzip (which should work!)
        // - We extract several line ranges and compare them with the help of head/tail with the original data
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_EXTRACTOR_AND_EXTRACT_SMALL_TO_COUT);

        path fastq = res.getResource(TEST_FASTQ_SMALL);
        path index = res.filePath(TEST_INDEX_SMALL);
        path extractedFastq = res.filePath("test.fastq");
        u_int64_t linesInFastq = 4000;
        vector<string> decompressedSourceContent;

        if (!initializeComplexTest(fastq, index, extractedFastq, -1, linesInFastq, &decompressedSourceContent))
            return;

        // Do work, though the tests for the larger files don't.
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 0, 4000, 4000);
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 2740, 2000, 1260);
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 2000, 2000, 2000);
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 1000, 2000, 2000);
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 3000, 5000, 1000);
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 4000, 5000, 0);
    }

    TEST (TEST_CREATE_EXTRACTOR_AND_EXTRACT_LARGE_TO_COUT) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_EXTRACTOR_AND_EXTRACT_LARGE_TO_COUT);

        path fastq = res.getResource(TEST_FASTQ_LARGE);
        path index = res.filePath(TEST_INDEX_LARGE);
        path extractedFastq = res.filePath("test2.fastq");
        u_int64_t linesInFastq = 160000;
        vector<string> decompressedSourceContent;

//        Indexer *indexer = new Indexer(make_shared<PathInputSource>(fastq), index, 1, true, true, false, true);
//        indexer->createIndex();
//        delete indexer;
//
//                CHECK(exists(index));

        if (!initializeComplexTest(fastq, index, extractedFastq, 1, linesInFastq, &decompressedSourceContent))
            return;

        runRangedExtractionTest(fastq, index, decompressedSourceContent, 0, 2740, 2740);
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 0, 160000, 160000);

        runRangedExtractionTest(fastq, index, decompressedSourceContent, 2740, 160000, 157260);
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 14223, 4000, 4000);
        for (u_int64_t i = 0, j = 0; i < 150000; i += 17500, j++) {
            runRangedExtractionTest(fastq, index, decompressedSourceContent, 2740 + i, 4000 + j, 4000 + j);
        }
        runRangedExtractionTest(fastq, index, decompressedSourceContent, 80000, 100000, 80000);

// Test this, if a bigger fastq test file is there.
//        fastq = res.getResource("test3.fastq.gz");
//        index = res.filePath("test3.fastq.gz.fqi_v1");
//        extractedFastq = res.filePath("test3.fastq");
//        linesInFastq = 1600000;
//
//        runComplexDecompressionTest(fastq, index, extractedFastq, 16, linesInFastq);
    }

    TEST (TEST_CREATE_EXTRACTOR_AND_EXTRACT_CONCAT_TO_COUT) {
        TestResourcesAndFunctions res(INDEXER_SUITE_TESTS, TEST_CREATE_EXTRACTOR_AND_EXTRACT_CONCAT_TO_COUT);

        path fastq = res.getResource(TEST_FASTQ_SMALL);
        path fastqConcat = res.filePath("test_concat.fastq.gz");
        path index = res.filePath("test_concat.fastq.gz.fqi_v1");
        path extractedFastq = res.filePath("test_concat.fastq");
        vector<string> decompressedSourceContent;

        int appendCount = 4;
        u_int64_t linesInFastq = appendCount * 4000;

        bool result = TestResourcesAndFunctions::createConcatenatedFile(fastq, fastqConcat, appendCount);
                CHECK_EQUAL(true, result);

        if (!initializeComplexTest(fastqConcat, index, extractedFastq, 4, linesInFastq, &decompressedSourceContent))
            return;

        runRangedExtractionTest(fastqConcat, index, decompressedSourceContent, 3000, 5000, 5000);
        runRangedExtractionTest(fastqConcat, index, decompressedSourceContent, 10000, 4000, 4000);
        runRangedExtractionTest(fastqConcat, index, decompressedSourceContent, 12000, 4000, 4000);
        runRangedExtractionTest(fastqConcat, index, decompressedSourceContent, 15000, 4000, 1000);
        runRangedExtractionTest(fastqConcat, index, decompressedSourceContent, 16000, 4000, 0);
    }

}