/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../../src/common/ErrorMessages.h"
#include "../../src/runners/ActualRunner.h"
#include "../../src/runners/ExtractorRunner.h"
#include "../../src/runners/IndexerRunner.h"
#include "../../src/process/io/PathSink.h"
#include "../../src/process/io/StreamSource.h"
#include "../TestConstants.h"
#include "../TestResourcesAndFunctions.h"
#include <cstdio>
#include <experimental/filesystem>
#include <iostream>
#include <UnitTest++/UnitTest++.h>

using std::experimental::filesystem::path;

const char *TEST_FILEPATH_HELPER_METHOD = "testFilePathHelperMethod";
const char *TEST_CHECK_PREMISES_WITH_ALLOWED_PIPE = "Test for checkPremises with a piped (and allowed) input file";
const char *TEST_CHECK_PREMISES_WITH_FORBIDDEN_PIPE = "Test for checkPremises with a forbidden input pipe (extractor)";
const char *TEST_CHECK_PREMISES_WITH_FASTQ = "testCheckPremisesWithFastq";
const char *TEST_CHECK_PREMISES_WITH_MISSING_FASTQ = "testCheckPremisesWithMissingFastq";
const char *TEST_CHECK_PREMISES_WITH_EXISTING_INDEX = "testCheckPremisesWithExistingIndex";
const char *TEST_CHECK_PREMISES_WITH_FASTQ_BEHIND_SYMLINK_CHAIN = "testCheckPremisesWithFastqBehindSymlinkChain";
const char *TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN = "testCheckPremisesWithUnreadbleFastqBehindSymlinkChain";

const char *SUITE_ID = "RunnerTests";

path fastqFile(TestResourcesAndFunctions *res) {
    return res->filePath(FASTQ_FILENAME);
}

shared_ptr<PathSource> _fastqFile(TestResourcesAndFunctions *res) {
    return PathSource::from(fastqFile(res));
}

path indexFile(TestResourcesAndFunctions *res) {
    return res->filePath(INDEX_FILENAME);
}

shared_ptr<PathSink> _indexFile(TestResourcesAndFunctions *res) {
    return shared_ptr<PathSink>(new PathSink(indexFile(res)));
}

SUITE (SUITE_ID) {

    TEST (TEST_FILEPATH_HELPER_METHOD) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_FILEPATH_HELPER_METHOD);
                CHECK(fastqFile(&res) == res.getTestPath().string() + "/" + FASTQ_FILENAME);
                CHECK(indexFile(&res) == res.getTestPath().string() + "/" + INDEX_FILENAME);
    }

    TEST (TEST_CHECK_PREMISES_WITH_ALLOWED_PIPE) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_FASTQ);
        IndexerRunner runner(make_shared<StreamSource>(&cin), make_shared<PathSink>(indexFile(&res)), BlockDistanceStorageStrategy::getDefault());
        bool result = runner.checkPremises();
                CHECK(result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_FASTQ) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_FASTQ);
        path fastq = fastqFile(&res);
        res.createEmptyFile(FASTQ_FILENAME);
        IndexerRunner runner(_fastqFile(&res), _indexFile(&res),make_shared<BlockDistanceStorageStrategy>(-1, true));
        bool result = runner.checkPremises();
                CHECK(result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_MISSING_FASTQ) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_MISSING_FASTQ);
        IndexerRunner *runner = new IndexerRunner(_fastqFile(&res), _indexFile(&res), BlockDistanceStorageStrategy::getDefault());
        bool result = runner->checkPremises();
                CHECK(!result);
//                CHECK(runner->getErrorMessages().size() == 1);
//                CHECK(runner->getErrorMessages()[0] == ERR_MESSAGE_FASTQ_INVALID);
        delete runner;
    }

    TEST (TEST_CHECK_PREMISES_WITH_FASTQ_BEHIND_SYMLINK_CHAIN) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_FASTQ_BEHIND_SYMLINK_CHAIN);
        path fastq = fastqFile(&res);
        res.createEmptyFile(FASTQ_FILENAME);
        path symlink1 = path(fastq.string() + ".link1");
        path symlink2 = path(symlink1.string() + ".link2");
        path symlink3 = path(symlink2.string() + ".link3");
        create_symlink(fastq, symlink1);
        create_symlink(symlink1, symlink2);
        create_symlink(symlink2, symlink3);
        IndexerRunner runner(shared_ptr<Source>(new PathSource(symlink3)), _indexFile(&res), BlockDistanceStorageStrategy::getDefault());
        bool result = runner.checkPremises();
                CHECK_EQUAL(true, result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN);
        path fastq = fastqFile(&res);
        path symlink1 = path(fastq.string() + ".link1");
        create_symlink(fastq, symlink1);
        IndexerRunner runner(shared_ptr<Source>(new PathSource(symlink1)), _indexFile(&res), BlockDistanceStorageStrategy::getDefault());
        bool result = runner.checkPremises();
                CHECK_EQUAL(false, result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_EXISTING_INDEX) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_EXISTING_INDEX);
        res.createEmptyFile(FASTQ_FILENAME);
        res.createEmptyFile(INDEX_FILENAME);
        IndexerRunner runner(_fastqFile(&res), _indexFile(&res), BlockDistanceStorageStrategy::getDefault());
        bool result = runner.checkPremises();
        // It is not allowed to override an existing file! Test has to "fail"
                CHECK(!result);
    }
}