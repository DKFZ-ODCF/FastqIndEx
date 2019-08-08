/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/PathSink.h"
#include "process/io/PathSource.h"
#include "runners/IndexerRunner.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *INDEXERRUNNER_SUITE_TESTS = "IndexerRunnerTests";
const char *TEST_INDEXERRUNNER_CREATION = "IndexerRunnerCreation";
const char *TEST_INDEXERRUNNER_ERRORMESSAGE_PASSTHROUGH = "IndexerRunnerWithErrorMessagPassthrough";

SUITE (INDEXERRUNNER_SUITE_TESTS) {
    TEST (TEST_INDEXERRUNNER_CREATION) {
        TestResourcesAndFunctions res(INDEXERRUNNER_SUITE_TESTS, TEST_INDEXERRUNNER_CREATION);
        path fastq = res.createEmptyFile("fastq.fastq");
        path index = res.filePath("fastq.fastq.fqi");

        auto r = make_shared<IndexerRunner>(make_shared<PathSource>(fastq), make_shared<PathSink>(index), BlockDistanceStorageStrategy::getDefault());
                CHECK(r->fulfillsPremises()); // Index file missing.
                CHECK(!r->isCLIOptionsPrinter());
                CHECK(!r->isExtractor());
                CHECK(r->isIndexer());
                CHECK(r->getErrorMessages().empty());
    }

    TEST (TEST_INDEXERRUNNER_ERRORMESSAGE_PASSTHROUGH) {
        TestResourcesAndFunctions res(INDEXERRUNNER_SUITE_TESTS, TEST_INDEXERRUNNER_ERRORMESSAGE_PASSTHROUGH);
        path fastq = res.createEmptyFile("fastq.fastq");
        path index = res.createEmptyFile("fastq.fastq.fqi");

        auto r = make_shared<IndexerRunner>(make_shared<PathSource>(fastq), make_shared<PathSink>(index), BlockDistanceStorageStrategy::getDefault());
                CHECK(!r->fulfillsPremises()); // Index exists. Not indexable!
                CHECK(!r->isCLIOptionsPrinter());
                CHECK(!r->isExtractor());
                CHECK(r->isIndexer());
                CHECK(!r->getErrorMessages().empty());
    }
}