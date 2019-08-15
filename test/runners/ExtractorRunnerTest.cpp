/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../../src/runners/ExtractorRunner.h"
#include "process/io/FileSource.h"
#include "process/io/ConsoleSink.h"
#include "../TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *SUITE_EXTRACTORRUNNER_TESTS = "ExtractorRunnerTests";
const char *TEST_EXTRACTORRUNNER_CREATION = "ExtractorRunner creation";
const char *TEST_EXTRACTORRUNNER_CREATION_VALID_FILES = "ExtractorRunner creation with valid files";

SUITE (SUITE_EXTRACTORRUNNER_TESTS) {
    TEST (TEST_EXTRACTORRUNNER_CREATION) {
        TestResourcesAndFunctions res(SUITE_EXTRACTORRUNNER_TESTS, TEST_EXTRACTORRUNNER_CREATION);

        path fastq = res.createEmptyFile("fastq.fastq");
        path index = res.createEmptyFile("fastq.fastq.fqi");

        ExtractorRunner r(make_shared<FileSource>(fastq),
                          make_shared<FileSource>(index),
                          ConsoleSink::create(),
                          false, ExtractMode::lines, 0, 100, 0, false);
                CHECK(!r.fulfillsPremises()); // Index is empty. Won't work!
                CHECK(!r.isCLIOptionsPrinter());
                CHECK(r.isExtractor());
                CHECK(!r.isIndexer());
                CHECK(!r.getErrorMessages().empty());
    }

    TEST (TEST_EXTRACTORRUNNER_CREATION_VALID_FILES) {
        TestResourcesAndFunctions res(SUITE_EXTRACTORRUNNER_TESTS, TEST_EXTRACTORRUNNER_CREATION_VALID_FILES);

        path fastq = TestResourcesAndFunctions::getResource(TEST_FASTQ_SMALL);
        path index = TestResourcesAndFunctions::getResource(TEST_INDEX_SMALL);

        ExtractorRunner r(make_shared<FileSource>(fastq),
                          make_shared<FileSource>(index),
                          ConsoleSink::create(),
                          false, ExtractMode::lines, 0, 100, 0, false);
        bool premisesFulfilled = r.fulfillsPremises();
                CHECK(premisesFulfilled);
                CHECK(!r.isCLIOptionsPrinter());
                CHECK(r.isExtractor());
                CHECK(!r.isIndexer());
                CHECK(r.getErrorMessages().empty());
    }
}