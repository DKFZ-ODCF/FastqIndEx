/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../../src/common/StringHelper.h"
#include <UnitTest++/UnitTest++.h>

const char *const STRINGHELPER_TESTS = "Test suite for IOHelper class";

const char *const TEST_SPLIT_STR = "Test splitStr()";

SUITE (STRINGHELPER_TESTS) {

    TEST (TEST_SPLIT_STR) {
        auto text = string("one\ntwo\nthree\nfour\n5\n6\n7\n\n");
        vector<string> expectedVector{
                "one", "two", "three", "four", "5", "6", "7", ""
        };
        vector<string> res = StringHelper::splitStr(text);
                CHECK_EQUAL(expectedVector.size(), res.size());
                CHECK_ARRAY_EQUAL(expectedVector, res, res.size());
    }
}