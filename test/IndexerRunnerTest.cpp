/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/IndexerRunner.h"
#include "../src/PathInputSource.h"
#include "../src/Runner.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *INDEXERRUNNER_SUITE_TESTS = "IndexerRunnerTests";
const char *TEST_INDEXERRUNNER_CREATION = "IndexerRunnerCreation";
const char *TEST_INDEXERRUNNER_ERRORMESSAGE_PASSTHROUGH = "IndexerRunnerWithErrorMessagPassthrough";

shared_ptr<IndexerRunner> createRunner(const char *const testID, bool createIndex) {
    TestResourcesAndFunctions res(INDEXERRUNNER_SUITE_TESTS, testID);

    path fastq = res.createEmptyFile("fastq.fastq");
    if (createIndex)
        return make_shared<IndexerRunner>(make_shared<PathInputSource>(fastq), res.createEmptyFile("fastq.fastq.fqi"));
    else
        return make_shared<IndexerRunner>(make_shared<PathInputSource>(fastq), res.filePath("fastq.fastq.fqi"));
}

SUITE (INDEXERRUNNER_SUITE_TESTS) {
    TEST (TEST_INDEXERRUNNER_CREATION) {
        TestResourcesAndFunctions res(INDEXERRUNNER_SUITE_TESTS, TEST_INDEXERRUNNER_CREATION);
        path fastq = res.createEmptyFile("fastq.fastq");
        path index = res.filePath("fastq.fastq.fqi");

        auto r = make_shared<IndexerRunner>(make_shared<PathInputSource>(fastq), index);
                CHECK(r->checkPremises()); // Index file missing.
                CHECK(!r->isCLIOptionsPrinter());
                CHECK(!r->isExtractor());
                CHECK(r->isIndexer());
                CHECK(r->getErrorMessages().empty());
    }

    TEST (TEST_INDEXERRUNNER_ERRORMESSAGE_PASSTHROUGH) {
        TestResourcesAndFunctions res(INDEXERRUNNER_SUITE_TESTS, TEST_INDEXERRUNNER_ERRORMESSAGE_PASSTHROUGH);
        path fastq = res.createEmptyFile("fastq.fastq");
        path index = res.createEmptyFile("fastq.fastq.fqi");

        auto r = make_shared<IndexerRunner>(make_shared<PathInputSource>(fastq), index);
                CHECK(!r->checkPremises()); // Index exists. Not indexable!
                CHECK(!r->isCLIOptionsPrinter());
                CHECK(!r->isExtractor());
                CHECK(r->isIndexer());
                CHECK(!r->getErrorMessages().empty());
    }
}