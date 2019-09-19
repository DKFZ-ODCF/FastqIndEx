/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/ErrorAccumulator.h"
#include "common/IOHelper.h"
#include "common/StringHelper.h"
#include "TestResourcesAndFunctions.h"
#include <experimental/filesystem>
#include <UnitTest++/UnitTest++.h>

using namespace std::experimental::filesystem;

const char *const IOHELPER_TESTS = "Test suite for IOHelper class";

const char *const TEST_REPORT_WITH_ACCUMULATOR = "Test report with accumulator";
const char *const TEST_REPORT_WITHOUT_ACCUMULATOR = "Test report without accumulator";

const char *const TEST_GETUSERHOMEDIRECTORY = "Test getUserHomeDirectory()";

const char *const TEST_CREATE_TEMPDIR = "Test createTempDir()";
const char *const TEST_CREATE_TEMPFILE = "Test createTempFile()";
const char *const TEST_CREATE_TEMPFIFO = "Test createTempFifo()";

const char *const TEST_CHECK_FILE_READABILITY_FAIL_NOTEXISTING = "Test check file readability (existance)";
const char *const TEST_CHECK_FILE_READABILITY_FAIL_NOTREADABLE = "Test check file readability (access)";
const char *const TEST_CHECK_FILE_READABILITY_SUCCEED = "Test check file readability";

const char *const TEST_CHECK_FILE_WRITEABILITY_FAIL_NOTEXISTING = "Test check file writeability (existance)";
const char *const TEST_CHECK_FILE_WRITEABILITY_FAIL_NOTREADABLE = "Test check file writeability (access)";
const char *const TEST_CHECK_FILE_WRITEABILITY_SUCCEED = "Test check file writeability";

const char *const TEST_FULLPATH = "Test fullpath()";
const char *const TEST_FULLPATH_WITH_HOME_SHORTCUT = "Test fullpath() with home shortcut (tilde)";

SUITE (IOHELPER_TESTS) {

    TEST (TEST_REPORT_WITH_ACCUMULATOR) {
        ErrorAccumulator accumulator;

                CHECK(accumulator.getErrorMessages().empty());

        IOHelper::report(stringstream("abc"), &accumulator);
                CHECK(accumulator.getErrorMessages().size() == 1);
    }

    TEST (TEST_REPORT_WITHOUT_ACCUMULATOR) {
        // It is not possible to test whether something was written to cerr or not. So just call report and see to it,
        // that it runs.
        IOHelper::report(stringstream("abc"), nullptr);
    }

    TEST (TEST_GETUSERHOMEDIRECTORY) {
        // ? How to test this properly?
                CHECK(IOHelper::getUserHomeDirectory().string() != "");
    }

    TEST (TEST_CREATE_TEMPDIR) {
        auto[success, tempDir, message] = IOHelper::createTempDir("createTempDirTest");
                CHECK(success);
                CHECK(exists(tempDir));
                CHECK(is_directory(tempDir));
        if (success)
            remove(tempDir);
                CHECK(!exists(tempDir));
    }

    TEST (TEST_CREATE_TEMPFILE) {
        auto[success, tempFile, message] = IOHelper::createTempFile("createTempFileTest");
                CHECK(success);
                CHECK(exists(tempFile));
                CHECK(is_regular_file(tempFile));
        if (success)
            remove(tempFile);
                CHECK(!exists(tempFile));
    }

    TEST (TEST_CREATE_TEMPFIFO) {
        auto[success, tempFifo, message] = IOHelper::createTempFifo("createTempFifoTest");
                CHECK(success);
                CHECK(exists(tempFifo));
                CHECK(is_fifo(tempFifo));
        if (success)
            remove(tempFifo);
                CHECK(!exists(tempFifo));
    }

    TEST (TEST_CHECK_FILE_READABILITY_FAIL_NOTEXISTING) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_CHECK_FILE_READABILITY_FAIL_NOTEXISTING);

        auto file = res.filePath("aMissingFile.txt");
        ErrorAccumulator accumulator;

                CHECK(!IOHelper::checkFileReadability(file, "something", &accumulator));
        const vector<string> &messages = accumulator.getErrorMessages();
        stringstream expectedError;
        expectedError << "The something file '" << file.string() << "' could not be found or is inaccessible.";
                CHECK(messages.size() == 1);
                CHECK(messages[0] == expectedError.str());
    }

    TEST (TEST_CHECK_FILE_READABILITY_FAIL_NOTREADABLE) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_CHECK_FILE_READABILITY_FAIL_NOTREADABLE);

        auto file = res.filePath("aMissingFile.txt");
        ErrorAccumulator accumulator;

                CHECK(!IOHelper::checkFileReadability(file, "something", &accumulator));
        const vector<string> &messages = accumulator.getErrorMessages();
                CHECK(messages.size() == 1);
    }

    TEST (TEST_CHECK_FILE_READABILITY_SUCCEED) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_CHECK_FILE_READABILITY_SUCCEED);

        auto file = TestResourcesAndFunctions::getResource("TestTextFile.txt");
        ErrorAccumulator accumulator;

                CHECK(IOHelper::checkFileReadability(file, "something", &accumulator));
                CHECK(accumulator.getErrorMessages().empty());
    }

    TEST (TEST_FULLPATH) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_FULLPATH);

        path l("file.txt");

                CHECK_EQUAL(current_path().string() + "/file.txt", IOHelper::fullPath(l).string());
    }

    TEST (TEST_FULLPATH_WITH_HOME_SHORTCUT) {
        TestResourcesAndFunctions res(IOHELPER_TESTS, TEST_FULLPATH_WITH_HOME_SHORTCUT);

        path l("~/file.txt");

                CHECK_EQUAL(IOHelper::getUserHomeDirectory().string() + "/file.txt", IOHelper::fullPath(l).string());
    }
}