/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/Runner.h"
#include "../src/Indexer.h"
#include "../src/IndexerRunner.h"
#include "../src/IndexReader.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <fstream>
#include <memory>
#include <zlib.h>
#include <UnitTest++/UnitTest++.h>

const char *const INTEGRATIONTESTS_SUITE = "IntegrationTests";
const char *const TEST_INDEXER_EXECUTION = "Run the indexer";
const char *const TEST_EXTRACTOR_EXECUTION = "Run the extractor";

SUITE (INTEGRATIONTESTS_SUITE) {
    TEST (TEST_INDEXER_EXECUTION) {
        TestResourcesAndFunctions res(INTEGRATIONTESTS_SUITE, TEST_INDEXER_EXECUTION);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.fqi");

        path pTestApp(TEST_BINARY);
        path pSrcApplication(pTestApp.parent_path().parent_path().string() + "/src");
        path pFastqIndex(pSrcApplication.string() + "/fastqindex");

//        int success = std::system("make");
//                CHECK_EQUAL(0, success);
//
        string command = (
                '"' + pFastqIndex.string() + '"' +
                " index" +
                " \"-f=" + fastq.string() + '"' +
                " \"-i=" + index.string() + '"'
        );

        int success = std::system(command.c_str());
                CHECK_EQUAL(0, success);
    }
}