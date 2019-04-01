/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>
#include <experimental/filesystem>

using namespace std::experimental::filesystem;

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
        TestResourcesAndFunctions res("ASimpleTest");

        path testPath = res.getTestPath();
        bool creationWasSuccessful = res.getTestPathCreationWasSuccessful();
                CHECK(!testPath.empty());
                CHECK(creationWasSuccessful);
                CHECK(exists(testPath));

        const string &fullPath = testPath.string();
                CHECK(fullPath.find(res.getTestSuite()) != std::string::npos);
                CHECK(fullPath.find(res.getTestName()) != std::string::npos);

        // Initially I had this test with a pointer to TestRes...Functions. Unfortunately, this raised a sigabort
        // That is, why I implemented the finalize() method, which will be called by the destructor.
        // delete res;
        res.finalize();
                CHECK(!exists(testPath));
    }

    TEST (testFilePath) {
        TestResourcesAndFunctions res("testFilePath");
                CHECK(res.filePath("abc.def") == res.getTestPath().string() + "/abc.def");
    }

    TEST (testGetResource) {
        TestResourcesAndFunctions res("testGetResource");
        // Utilizing current_path() will result in the path of current test-binary, which will be e.g.:
        //   ~/Projects/FastqIndEx/cmake-build-debug/test/testapp
        // As we do want resources to be loaded from:
        //   ~/Projects/FastqIndEx/cmake-build-debug/test/testapp
        // instead, we will have to walk a bit in the paths to get the right file.
        path expectedPath(
                current_path().parent_path().parent_path().string() + string("/test/resources/test2.fastq.gz"));
        path resourcePath = res.getResource("test2.fastq.gz");
                CHECK(exists(resourcePath));
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