/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/process/index/Indexer.h"
#include "../src/process/extract/IndexReader.h"
#include "../src/runners/IndexerRunner.h"
#include "../src/runners/Runner.h"
#include "../src/startup/Starter.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <fstream>
#include <memory>
#include <UnitTest++/UnitTest++.h>
#include <zlib.h>

const char *const INTEGRATIONTESTS_SUITE = "IntegrationTests";
const char *const TEST_INDEXER_EXECUTION = "Run the indexer";
const char *const TEST_PIPED_INDEXER_EXECUTION = "Run the indexer with a pipe";
const char *const TEST_EXTRACTOR_EXECUTION = "Run the extractor";
const char *const TEST_EXTRACTOR_EXECUTION_TO_FILE = "Run the extractor with an output file instead of cout";

SUITE (INTEGRATIONTESTS_SUITE) {
//    TEST (TEST_INDEXER_EXECUTION) {
//        TestResourcesAndFunctions res(INTEGRATIONTESTS_SUITE, TEST_INDEXER_EXECUTION);
//
//        path fastq = res.getResource(TEST_FASTQ_LARGE);
//        path index = res.filePath(TEST_INDEX_LARGE);
//
//        bool result = res.runIndexerBinary(fastq, index, false);
//                CHECK_EQUAL(true, result);
//    }
//
//    TEST (TEST_PIPED_INDEXER_EXECUTION) {
//        TestResourcesAndFunctions res(INTEGRATIONTESTS_SUITE, TEST_PIPED_INDEXER_EXECUTION);
//
//        path fastq = res.getResource(TEST_FASTQ_SMALL);
//        path refIndex = res.getResource(TEST_INDEX_SMALL);
//        path index = res.filePath(TEST_INDEX_SMALL);
//
//        Starter::getInstance()->createRunner()
////        bool result = res.runIndexerBinary(fastq, index, true);
////                CHECK(result);
////
////                CHECK(exists(index));
////
////        if (exists(index)) {
////                    CHECK_EQUAL(file_size(refIndex), file_size(index));
////             Compare file size && md5sum.
////        }
//    }
//
//    TEST (TEST_EXTRACTOR_EXECUTION) {
//        TestResourcesAndFunctions res(INTEGRATIONTESTS_SUITE, TEST_EXTRACTOR_EXECUTION);
//
//        path fastq = res.getResource(TEST_FASTQ_SMALL);
//        path refIndex = res.getResource(TEST_INDEX_SMALL);
//        path index = res.filePath(TEST_INDEX_SMALL);
//
//                CHECK(false);
//    }
//
//    TEST (TEST_EXTRACTOR_EXECUTION_TO_FILE) {
//        TestResourcesAndFunctions res(INTEGRATIONTESTS_SUITE, TEST_EXTRACTOR_EXECUTION_TO_FILE);
//
//                CHECK(false);
//    }
}