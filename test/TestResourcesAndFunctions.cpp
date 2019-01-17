/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <cstdio>
#include <cstring>
#include "TestResourcesAndFunctions.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace boost::filesystem;

TestResourcesAndFunctions::TestResourcesAndFunctions(string testSuite, string testName) {
    this->testSuite = move(testSuite);
    this->testName = move(testName);
    if (this->testSuite.empty())
        this->testSuite = "default";
}

TestResourcesAndFunctions::TestResourcesAndFunctions(string testName) {
    this->testSuite = "default";
    this->testName = move(testName);
}

TestResourcesAndFunctions::~TestResourcesAndFunctions() {
    lock.lock();
    if (!testPath.empty())
        remove_all(testPath);
    testPath = "";
    lock.unlock();
}

path TestResourcesAndFunctions::getTestPath() {
    lock.lock();
    if (testPath.empty()) {
        path tempPath = temp_directory_path();
        string namePattern =
                tempPath.string() + "/FastqInDexTest_" + testSuite + "_" + testName + "_%%%%-%%%%-%%%%-%%%%";
        testPath = unique_path(namePattern);
        testPathCreationWasSuccessful = create_directories(testPath);
    }
    lock.unlock();
    return testPath;
}

string TestResourcesAndFunctions::getTestSuite() {
    return testSuite;
}

string TestResourcesAndFunctions::getTestName() {
    return testName;
}

bool TestResourcesAndFunctions::getTestPathCreationWasSuccessful() {
    return testPathCreationWasSuccessful;
}

path TestResourcesAndFunctions::filePath(string filename) {
    return getTestPath().string() + "/" + filename;
}

path TestResourcesAndFunctions::getResource(string filename) {
    return path(current_path().string() + string("/resources/") + filename);
}

path TestResourcesAndFunctions::createEmptyFile(string filename) {
    path _path = filePath(move(filename));
    CreateEmptyFile(_path);
    return _path;
}

void TestResourcesAndFunctions::CreateEmptyFile(path _path) {
    boost::filesystem::ofstream stream(_path);
    stream << "";
    stream.close();
}