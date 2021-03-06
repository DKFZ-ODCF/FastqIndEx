/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "runners/DoNothingRunner.h"
#include "../TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char* SUITE_INDEXERRUNNER_TESTS = "PrintCLIOptionsRunnerTests";
const char* TEST_PRINTCLIOPTIONSRUNNER_CREATION = "PrintCLIOptionsRunnerCreation";

SUITE (SUITE_INDEXERRUNNER_TESTS) {
    TEST (TEST_INDEXERRUNNER_CREATION) {
        TestResourcesAndFunctions res(SUITE_INDEXERRUNNER_TESTS, TEST_PRINTCLIOPTIONSRUNNER_CREATION);
        DoNothingRunner r;
                CHECK(r.fulfillsPremises());
                CHECK(r.isCLIOptionsPrinter());
                CHECK(!r.isExtractor());
                CHECK(!r.isIndexer());
                CHECK(r.getErrorMessages().empty());
    }
}