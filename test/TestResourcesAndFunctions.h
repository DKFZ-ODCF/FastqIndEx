/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_TESTRESOURCESANDFUNCTIONS_H
#define FASTQINDEX_TESTRESOURCESANDFUNCTIONS_H

#include <string>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace boost::filesystem;

class TestResourcesAndFunctions {

private:

    string testSuite;

    string testName;

    path testPath;

    bool testPathCreationWasSuccessful;

    boost::mutex lock;

public:

    TestResourcesAndFunctions(string testSuite, string testName);

    explicit TestResourcesAndFunctions(string testName);

    virtual ~TestResourcesAndFunctions();

    string getTestSuite();

    string getTestName();

    path getTestPath();

    bool getTestPathCreationWasSuccessful();

    path filePath(string filename);

    path getResource(string filename);

    path createEmptyFile(string filename);

    static void CreateEmptyFile(path _path);
};


#endif //FASTQINDEX_TESTRESOURCESANDFUNCTIONS_H
