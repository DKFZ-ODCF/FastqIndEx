/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "TestResourcesAndFunctions.h"
#include <cstdio>
#include <cstring>
#include <experimental/filesystem>
#include <iostream>
#include <UnitTest++/UnitTest++.h>
#include <common/IOHelper.h>

using namespace std::experimental::filesystem;
using std::experimental::filesystem::path;

mutex TestResourcesAndFunctions::staticLock;

vector<string> TestResourcesAndFunctions::testVectorWithSimulatedDecompressedBlockData;

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
    finalize();
}

void TestResourcesAndFunctions::finalize() {
    lock.lock();
    if (!testPath.empty())
        remove_all(testPath);
    testPath = "";
    lock.unlock();
}

path TestResourcesAndFunctions::getTestPath() {
    lock.lock();
    if (testPath.empty()) {
        string prefix = "FastqIndExTest_" + testSuite + "_" + testName;
        auto[result, tempDir] = IOHelper::createTempDir(prefix);
        if (!result) {
            cerr << "## The temporary folder for pattern '" << prefix << "' could not be created!\n";
        }
        bool success = exists(tempDir);
                CHECK(!tempDir.string().empty());
                CHECK(success);
        testPathCreationWasSuccessful = success;
        testPath = tempDir;
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

path TestResourcesAndFunctions::filePath(const string &filename) {
    return getTestPath().string() + "/" + filename;
}

path TestResourcesAndFunctions::getResource(const string &filename) {
    // Utilizing current_path() will result in the path of current test-binary, which will be e.g.:
    //   ~/Projects/FastqIndEx/cmake-build-debug/test/testapp
    // As we do want resources to be loaded from:
    //   ~/Projects/FastqIndEx/cmake-build-debug/test/testapp
    // instead, we will have to walk a bit in the paths to get the right file.
    path applicationBasePath = current_path().parent_path().parent_path();
    path resourcePath = path(applicationBasePath.string() + string("/test/resources/") + filename);
    if (!exists(resourcePath))
        cout << "The resource " << applicationBasePath;
    return resourcePath;
}

path TestResourcesAndFunctions::createEmptyFile(const string &filename) {
    path _path = filePath(filename);
    CreateEmptyFile(_path);
    return _path;
}

void TestResourcesAndFunctions::CreateEmptyFile(const path &_path) {

    ofstream streamTest("/tmp/sometestfile");
    streamTest << "Something";
    ofstream stream(_path);
    stream << "";
    stream.close();
}

std::string TestResourcesAndFunctions::format(std::string format, ...) {
    va_list args;
    va_start(args, format);
    size_t len = std::vsnprintf(nullptr, 0, format.c_str(), args);// TODO
    va_end(args);
    std::vector<char> vec(len + 1);
    va_start(args, format);
    std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
    va_end(args);
    return &vec[0];
}

path TestResourcesAndFunctions::getPathOfFQIBinary() {
    path pTestApp(TEST_BINARY);
    path pSrcApplication(pTestApp.parent_path().parent_path().string() + "/src");
    path pFastqIndex(pSrcApplication.string() + "/fastqindex");
    return pFastqIndex;
}

bool TestResourcesAndFunctions::runCommand(const string &command) {
    return std::system(command.c_str()) == 0;
}

/**
 * No test for this, tested with integration tests.
 */
bool TestResourcesAndFunctions::runIndexerBinary(const path &fastq, const path &index, bool pipeFastq) {
    if (!pipeFastq) {
        return runCommand(
                format(R"("%s" index "-f=%s" "-i=%s")",
                       getPathOfFQIBinary().string().c_str(),
                       fastq.string().c_str(),
                       index.string().c_str()
                )
        );
    } else {
        return runCommand(
                format(R"(cat "%s" | "%s" index -f=- -i="%s")",
                       fastq.string().c_str(),
                       getPathOfFQIBinary().string().c_str(),
                       index.string().c_str()
                )
        );
    }
}

/**
 * No test for this, tested with integration tests.
 */
bool TestResourcesAndFunctions::runExtractorBinary(const path &fastq, const path &index) {
    return runCommand(
            format(R"("%s" extract "-f=%s" "-i=%s")",
                   getPathOfFQIBinary().string().c_str(),
                   fastq.string().c_str(),
                   index.string().c_str())
    );
}

/**
 * No test for this, tested within other tests.
 */
bool TestResourcesAndFunctions::extractGZFile(const path &file, const path &extractedFile) {
    return runCommand(
            format(string(R"(gunzip -c "%s" > "%s")"), file.string().c_str(), extractedFile.string().c_str())
    );
}

/**
 * No test for this, tested within other tests.
 */
bool TestResourcesAndFunctions::createConcatenatedFile(const path &file, const path &result, int repetitions) {
            CHECK(repetitions > 0);
    bool res = true;
    /**
     * First time, create a fresh file.
     */
    res &= runCommand(
            format(R"(cat "%s" > "%s")",
                   file.string().c_str(),
                   result.string().c_str()
            )
    );
    /**
     * Second to n-time append gz.
     */
    for (int i = 1; i < repetitions; i++) {
        res &= runCommand(
                format(R"(cat "%s" >> "%s")",
                       file.string().c_str(),
                       result.string().c_str()
                )
        );
    }
    return res;
}

vector<string> TestResourcesAndFunctions::readLinesOfFile(const path &file) {
    ifstream strm(file);
    vector<string> decompressedSourceContent;
    string line;
    while (std::getline(strm, line)) {
        decompressedSourceContent.emplace_back(line);
    }
    return decompressedSourceContent;
}

string TestResourcesAndFunctions::readFile(const path &file) {
    std::ifstream t(file);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());// TODO
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return str;
}

bool TestResourcesAndFunctions::compareVectorContent(const vector<string> &reference, const vector<string> &actual,
                                                     uint32_t referenceOffset) {
    int64_t firstDiff{-1};
    for (u_int64_t i = 0; i < std::min(reference.size() - referenceOffset, actual.size()); i++) {
        if (reference[i + referenceOffset] != actual[i]) {
            firstDiff = i;
            break;
        }
    }
    return firstDiff == -1;
}

const vector<string> &TestResourcesAndFunctions::getTestVectorWithSimulatedBlockData() {
    lock_guard<mutex> lock(TestResourcesAndFunctions::staticLock);

    if (!testVectorWithSimulatedDecompressedBlockData.empty())
        return testVectorWithSimulatedDecompressedBlockData;

    vector<string> *v = &testVectorWithSimulatedDecompressedBlockData;

    v->emplace_back(readResourceFile("blockAndLineCalculations/block_0_complete"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_1_endopen"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_2_startendopen"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_5_empty"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_5_empty"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_3_startendopen"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_3.1_startendopen_nonewlines"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_4_startopen"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_5_empty"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_6_newlines"));
    v->emplace_back(readResourceFile("blockAndLineCalculations/block_7_finalwonewlines"));

    return testVectorWithSimulatedDecompressedBlockData;
}
