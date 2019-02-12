/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/ExtractorRunner.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char* SUITE_EXTRACTORRUNNER_TESTS = "ExtractorRunnerTests";
const char* TEST_EXTRACTORRUNNER_CREATION = "ExtractorRunnerCreation";

SUITE (SUITE_EXTRACTORRUNNER_TESTS) {
    TEST (TEST_EXTRACTORRUNNER_CREATION) {
        TestResourcesAndFunctions res(SUITE_EXTRACTORRUNNER_TESTS, TEST_EXTRACTORRUNNER_CREATION);

        path fastq = res.createEmptyFile("fastq.fastq");
        path index = res.createEmptyFile("fastq.fastq.idx");

        ExtractorRunner r(fastq, index, 0, 100);
                CHECK(r.checkPremises());
                CHECK(!r.isCLIOptionsPrinter());
                CHECK(r.isExtractor());
                CHECK(!r.isIndexer());
                CHECK(r.getErrorMessages().empty());
    }
}