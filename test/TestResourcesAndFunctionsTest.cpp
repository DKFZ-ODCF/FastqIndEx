/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

SUITE (TestResourcesAndFunctionsTest) {

    TEST (testConstructors) {
        TestResourcesAndFunctions withSuite("aSuite", "testConstructors");
                CHECK(withSuite.getTestSuite() == "aSuite");
                CHECK(withSuite.getTestName() == "testConstructors");
        TestResourcesAndFunctions withoutSuite("testConstructors");
                CHECK(withoutSuite.getTestSuite() == "default");
        TestResourcesAndFunctions withEmptySuite("", "testConstructors");
                CHECK(withEmptySuite.getTestSuite() == "default");
    }

    TEST (testCreationAndDestructionWithTestPath) {
        TestResourcesAndFunctions *res = new TestResourcesAndFunctions("ASimpleTest");

        path testPath = res->getTestPath();
                CHECK(!testPath.empty());
                CHECK(res->getTestPathCreationWasSuccessful());
                CHECK(exists(testPath));

        const string &fullPath = testPath.string();
                CHECK(fullPath.find(res->getTestSuite()) != std::string::npos);
                CHECK(fullPath.find(res->getTestName()) != std::string::npos);

        delete res;
                CHECK(!exists(testPath));
    }

    TEST (testFilePath) {
        TestResourcesAndFunctions res("testFilePath");
                CHECK(res.filePath("abc.def") == res.getTestPath().string() + "/abc.def");
    }

    TEST (testGetResource) {
        TestResourcesAndFunctions res("testGetResource");
        path expectedPath(current_path().string() + string("/resources/test2.fastq.gz"));
        path resourcePath = res.getResource("test2.fastq.gz");
                CHECK(expectedPath == resourcePath);
    }

    TEST (testCreateEmptyFile) {
        // Also tests the static function.
        TestResourcesAndFunctions res("testCreateEmptyFile");

        path _path = res.createEmptyFile(string("abc.def"));
                CHECK(_path == res.filePath("abc.def"));
                CHECK(exists(_path));
    }
}