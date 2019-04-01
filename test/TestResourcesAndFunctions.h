/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_TESTRESOURCESANDFUNCTIONS_H
#define FASTQINDEX_TESTRESOURCESANDFUNCTIONS_H

#include <experimental/filesystem>
#include <mutex>
#include <string>

using namespace std;
using std::experimental::filesystem::path;


class TestResourcesAndFunctions {

private:

    string testSuite;

    string testName;

    path testPath = path("");

    bool testPathCreationWasSuccessful = false;

    mutex lock;

public:

    TestResourcesAndFunctions(string testSuite, string testName);

    explicit TestResourcesAndFunctions(string testName);

    virtual ~TestResourcesAndFunctions();

    /**
     * Initially I had this tested with a pointer to TestRes...Functions. Unfortunately, this raised a sigabort
     * That is, why I implemented the finalize() method, which will be called by the destructor.
     */
    void finalize();

    string getTestSuite();

    string getTestName();

    path getTestPath();

    bool getTestPathCreationWasSuccessful();

    path filePath(const string &filename);

    path getResource(const string &filename);

    path createEmptyFile(const string &filename);

    static void CreateEmptyFile(const path &_path);
};


#endif //FASTQINDEX_TESTRESOURCESANDFUNCTIONS_H
