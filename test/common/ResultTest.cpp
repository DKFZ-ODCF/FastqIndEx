/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/Result.h"
#include <UnitTest++/UnitTest++.h>
#include <string>

const char *const COMMONSTUFF_TESTS = "Test suite for Result struct";

const char *const TEST_CONSTRUCT = "Test constructor";
const char *const TEST_BOOL_OPERATOR = "Test operator bool()";

using namespace std;

Result<bool> testResultBoolConstruction() {
    return {true, false, ""};
}

SUITE (COMMONSTUFF_TESTS) {

    TEST (TEST_CONSTRUCT) {
        Result<bool> resultBool(true, false);
                CHECK_EQUAL(true, resultBool.success);
                CHECK_EQUAL(false, resultBool.result);
                CHECK_EQUAL("", resultBool.message);

        Result<bool> resultBool1(true, false, "abc");
                CHECK_EQUAL(true, resultBool1.success);
                CHECK_EQUAL(false, resultBool1.result);
                CHECK_EQUAL("abc", resultBool1.message);

        Result<string> resultString(true, "abc", "");
                CHECK_EQUAL("abc", resultString.result);

        Result<bool> resultWithMethod = testResultBoolConstruction();

        Result<bool> resultShort = {true, false, ""};

        Result<bool> resultCopy = resultShort;
    }

    TEST (TEST_BOOL_OPERATOR) {
        Result<bool> resultBool(true, false);
                CHECK(resultBool);

        Result<bool> resultBool1(false, false);
                CHECK(!resultBool1);
    }
}