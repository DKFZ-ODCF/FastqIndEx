/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "../src/Runner.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char* SUITE_INDEXERRUNNER_TESTS = "ShowStopperRunnerTests";
const char* TEST_SHOWSTOPPERRUNNER_CREATION = "ShowStopperRunnerCreation";

SUITE (SUITE_INDEXERRUNNER_TESTS) {
    TEST (TEST_INDEXERRUNNER_CREATION) {
        TestResourcesAndFunctions res(SUITE_INDEXERRUNNER_TESTS, TEST_SHOWSTOPPERRUNNER_CREATION);
        ShowStopperRunner r;
                CHECK(r.checkPremises());
                CHECK(r.isShowStopper());
                CHECK(!r.isExtractor());
                CHECK(!r.isIndexer());
                CHECK(r.getErrorMessages().empty());
    }
}