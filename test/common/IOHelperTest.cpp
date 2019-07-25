/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../../src/common/ErrorAccumulator.h"
#include "../../src/common/IOHelper.h"
#include "../../src/common/StringHelper.h"
#include "../TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *const IOHELPER_TESTS = "Test suite for IOHelper class";

const char *const TEST_SPLIT_STR = "Test splitStr()";

const char *const TEST_CHECK_FILE_READABILITY_FAIL_NOTEXISTING = "Test check file readability (existance)";
const char *const TEST_CHECK_FILE_READABILITY_FAIL_NOTREADABLE = "Test check file readability (access)";
const char *const TEST_CHECK_FILE_READABILITY_SUCCEED = "Test check file readability";

const char *const TEST_CHECK_FILE_WRITEABILITY_FAIL_NOTEXISTING = "Test check file writeability (existance)";
const char *const TEST_CHECK_FILE_WRITEABILITY_FAIL_NOTREADABLE = "Test check file writeability (access)";
const char *const TEST_CHECK_FILE_WRITEABILITY_SUCCEED = "Test check file writeability";

SUITE (IOHELPER_TESTS) {

    TEST (TEST_SPLIT_STR) {
        auto text = string("one\ntwo\nthree\nfour\n5\n6\n7\n\n");
        vector<string> expectedVector{
                "one", "two", "three", "four", "5", "6", "7", ""
        };
        vector<string> res = StringHelper::splitStr(text);
                CHECK_EQUAL(expectedVector.size(), res.size());
                CHECK_ARRAY_EQUAL(expectedVector, res, res.size());
    }

    TEST (TEST_CHECK_FILE_READABILITY_FAIL_NOTEXISTING) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_CHECK_FILE_READABILITY_FAIL_NOTEXISTING);

        auto file = res.filePath("aMissingFile.txt");
        ErrorAccumulator accumulator;

                CHECK(false == IOHelper::checkFileReadability(file, "something", &accumulator));
        const vector<string> &messages = accumulator.getErrorMessages();
        stringstream expectedError;
        expectedError << "The something file '" << file.string() << "' could not be found or is not accessible.";
                CHECK(messages.size() == 1);
                CHECK(messages[0] == expectedError.str());
    }

    TEST (TEST_CHECK_FILE_READABILITY_FAIL_NOTREADABLE) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_CHECK_FILE_READABILITY_FAIL_NOTREADABLE);

        auto file = res.filePath("aMissingFile.txt");
        ErrorAccumulator accumulator;

                CHECK(false == IOHelper::checkFileReadability(file, "something", &accumulator));
//        const vector<string> &messages = accumulator.getErrorMessages();
//        stringstream expectedError;
//        expectedError << "The 'something' file '" << file.string() << "' could not be found or is not accessible.";
//                CHECK(messages.size() == 1);
//                CHECK(messages[0] == expectedError.str());

                CHECK(false);
    }

    TEST (TEST_CHECK_FILE_READABILITY_SUCCEED) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_CHECK_FILE_READABILITY_SUCCEED);

        auto file = res.getResource("TestTextFile.txt");
        ErrorAccumulator accumulator;

                CHECK(true == IOHelper::checkFileReadability(file, "something", &accumulator));
                CHECK(accumulator.getErrorMessages().empty());
    }

    TEST(fullPath) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, "fullPath");

        path l("file.txt");

        CHECK_EQUAL(current_path().string() + "/file.txt", IOHelper::fullPath(l).string());
    }
}