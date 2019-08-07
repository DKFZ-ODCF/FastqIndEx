/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "runners/IndexStatsRunner.h"
#include "process/io/PathSource.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *SUITE_INDEXSTATSRUNNER_TESTS = "PrintCLIOptionsRunnerTests";
const char *TEST_INDEXSTATSRUNNER_CREATION = "PrintCLIOptionsRunnerCreation";

SUITE (SUITE_INDEXSTATSRUNNER_TESTS) {
    TEST (TEST_INDEXERRUNNER_CREATION) {
        TestResourcesAndFunctions res(SUITE_INDEXSTATSRUNNER_TESTS, TEST_INDEXSTATSRUNNER_CREATION);
        path p = res.getResource(TEST_INDEX_SMALL);
        IndexStatsRunner r(make_shared<PathSource>(p), 0, 1);
                CHECK(r.fulfillsPremises());
                CHECK(!r.isCLIOptionsPrinter());
                CHECK(!r.isExtractor());
                CHECK(!r.isIndexer());
                CHECK(r.getErrorMessages().empty());
                CHECK(0 == r.run());
    }
}