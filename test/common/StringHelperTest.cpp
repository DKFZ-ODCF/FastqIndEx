/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/CommonStructsAndConstants.h"
#include "common/StringHelper.h"
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

    TEST (TEST_BYTESTRATEGY_PARSESTRINGVALUE) {
        // Invalid values first => Will result in -1
                CHECK_EQUAL(-1, StringHelper::parseStringValue(""));
                CHECK_EQUAL(-1, StringHelper::parseStringValue("ab"));
                CHECK_EQUAL(-1, StringHelper::parseStringValue("3a"));
                CHECK_EQUAL(-1, StringHelper::parseStringValue("3.0k"));

        // Valid values
                CHECK_EQUAL(3 * MB, StringHelper::parseStringValue("3"));

                CHECK_EQUAL(3 * kB, StringHelper::parseStringValue("3k"));
                CHECK_EQUAL(3 * kB, StringHelper::parseStringValue("3K"));
                CHECK_EQUAL(3 * MB, StringHelper::parseStringValue("3m"));
                CHECK_EQUAL(3 * MB, StringHelper::parseStringValue("3M"));

                CHECK_EQUAL(3 * GB, StringHelper::parseStringValue("3g"));
                CHECK_EQUAL(3 * GB, StringHelper::parseStringValue("3G"));
                CHECK_EQUAL(3 * TB, StringHelper::parseStringValue("3t"));
                CHECK_EQUAL(3 * TB, StringHelper::parseStringValue("3T"));
    }
}