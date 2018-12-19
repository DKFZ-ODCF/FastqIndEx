/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "../src/Runner.h"
#include "../src/IndexerRunner.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const string INDEXERRUNNER_SUITE_TESTS = "IndexerRunnerTests";
const string TEST_INDEXERRUNNER_CREATION = "IndexerRunnerCreation";

SUITE (INDEXERRUNNER_SUITE_TESTS) {
    TEST (TEST_INDEXERRUNNER_CREATION) {
        TestResourcesAndFunctions res(INDEXERRUNNER_SUITE_TESTS, TEST_INDEXERRUNNER_CREATION);

        path fastq = res.createEmptyFile("fastq.fastq");
        path index = res.createEmptyFile("fastq.fastq.idx");

        IndexerRunner r(fastq, index);
                CHECK(r.checkPremises());
                CHECK(!r.isShowStopper());
                CHECK(!r.isExtractor());
                CHECK(r.isIndexer());
                CHECK(r.getErrorMessages().empty());
    }
}