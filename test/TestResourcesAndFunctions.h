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
#include <cstdarg>
#include <fstream>
#include "TestConstants.h"

using namespace std;
using std::experimental::filesystem::path;


class TestResourcesAndFunctions {

private:

    string testSuite;

    string testName;

    path testPath = path("");

    bool testPathCreationWasSuccessful = false;

    mutex lock;

    static mutex staticLock;

    static vector<string> testVectorWithSimulatedDecompressedBlockData;

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

    static path getResource(const string &filename);

    path createEmptyFile(const string &filename);

    static void CreateEmptyFile(const path &_path);

    /**
     * This method formats a string with parameters like %s, %d.
     *
     * The source of this method is here: http://www.martinbroadhurst.com/string-formatting-in-c.html
     *
     * This method is definitely not safe, it does not check, how many placeholders are in! It will cause a sigsev, if
     * e.g. more placeholders than arguments exist.
     *
     * @param format The string to format with placeholders like %s, %d.
     * @param ...    The range of parameters which should be put in place of the placeholders.
     * @return
     */
    static std::string format(std::string format, ...);

    static path getPathOfFQIBinary();

    static bool runCommand(const string &command);

    static bool runIndexerBinary(const path &fastq, const path &index, bool pipeFastq);

    static bool runExtractorBinary(const path &fastq, const path &index);

    static bool extractGZFile(const path &file, const path &extractedFile);

    static bool createConcatenatedFile(const path &file, const path &result, int repetitions);

    static vector<string> readLinesOfFile(const path &file);

    static string readFile(const path &file);

    vector<string> readLinesOfResourceFile(const string &resourceFile) {
        return readLinesOfFile(getResource(resourceFile));
    }

    static string readResourceFile(const string &resourceFile) {
        return readFile(getResource(resourceFile));
    }

    static bool
    compareVectorContent(const vector<string> &reference, const vector<string> &actual, uint32_t referenceOffset = 0);

    static const vector<string> &getTestVectorWithSimulatedBlockData();
};


#endif //FASTQINDEX_TESTRESOURCESANDFUNCTIONS_H
