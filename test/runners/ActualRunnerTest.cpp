/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../../src/common/ErrorMessages.h"
#include "../../src/runners/ExtractorRunner.h"
#include "../../src/runners/IndexerRunner.h"
#include "process/io/FileSink.h"
#include "../../src/process/io/StreamSource.h"
#include "../TestConstants.h"
#include "../TestResourcesAndFunctions.h"
#include <experimental/filesystem>
#include <iostream>
#include <memory>
#include <UnitTest++/UnitTest++.h>

using std::experimental::filesystem::path;

const char *TEST_FILEPATH_HELPER_METHOD = "testFilePathHelperMethod";
const char *TEST_CHECK_PREMISES_WITH_ALLOWED_PIPE = "Test for fulfillsPremises with a piped (and allowed) input file";
const char *TEST_CHECK_PREMISES_WITH_FORBIDDEN_PIPE = "Test for fulfillsPremises with a forbidden input pipe (extractor)";
const char *TEST_CHECK_PREMISES_WITH_FASTQ = "testCheckPremisesWithFastq";
const char *TEST_CHECK_PREMISES_WITH_MISSING_FASTQ = "testCheckPremisesWithMissingFastq";
const char *TEST_CHECK_PREMISES_WITH_EXISTING_INDEX = "testCheckPremisesWithExistingIndex";
const char *TEST_CHECK_PREMISES_WITH_FASTQ_BEHIND_SYMLINK_CHAIN = "testCheckPremisesWithFastqBehindSymlinkChain";
const char *TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN = "testCheckPremisesWithUnreadbleFastqBehindSymlinkChain";

const char *SUITE_ID = "RunnerTests";

path sourceFile(TestResourcesAndFunctions *res) {
    return res->filePath(FASTQ_FILENAME);
}

shared_ptr<FileSource> _sourceFile(TestResourcesAndFunctions *res) {
    return FileSource::from(sourceFile(res));
}

path indexFile(TestResourcesAndFunctions *res) {
    return res->filePath(INDEX_FILENAME);
}

shared_ptr<FileSink> _indexFile(TestResourcesAndFunctions *res) {
    return std::make_shared<FileSink>(indexFile(res));
}

SUITE (SUITE_ID) {

    TEST (TEST_FILEPATH_HELPER_METHOD) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_FILEPATH_HELPER_METHOD);
                CHECK(sourceFile(&res).string() == res.getTestPath().string() + "/" + FASTQ_FILENAME);
                CHECK(indexFile(&res).string() == res.getTestPath().string() + "/" + INDEX_FILENAME);
    }

    TEST (TEST_CHECK_PREMISES_WITH_ALLOWED_PIPE) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_FASTQ);
        IndexerRunner runner(make_shared<StreamSource>(&cin), make_shared<FileSink>(indexFile(&res)), BlockDistanceStorageDecisionStrategy::getDefault());
        bool result = runner.fulfillsPremises();
                CHECK(result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_FASTQ) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_FASTQ);
        path fastq = sourceFile(&res);
        res.createEmptyFile(FASTQ_FILENAME);
        IndexerRunner runner(_sourceFile(&res), _indexFile(&res),make_shared<BlockDistanceStorageDecisionStrategy>(-1, true));
        bool result = runner.fulfillsPremises();
                CHECK(result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_MISSING_FASTQ) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_MISSING_FASTQ);
        auto runner = new IndexerRunner(_sourceFile(&res), _indexFile(&res), BlockDistanceStorageDecisionStrategy::getDefault());
        bool result = runner->fulfillsPremises();
                CHECK(!result);
//                CHECK(runner->getErrorMessages().size() == 1);
//                CHECK(runner->getErrorMessages()[0] == ERR_MESSAGE_FASTQ_INVALID);
        delete runner;
    }

    TEST (TEST_CHECK_PREMISES_WITH_FASTQ_BEHIND_SYMLINK_CHAIN) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_FASTQ_BEHIND_SYMLINK_CHAIN);
        path fastq = sourceFile(&res);
        res.createEmptyFile(FASTQ_FILENAME);
        path symlink1 = path(fastq.string() + ".link1");
        path symlink2 = path(symlink1.string() + ".link2");
        path symlink3 = path(symlink2.string() + ".link3");
        create_symlink(fastq, symlink1);
        create_symlink(symlink1, symlink2);
        create_symlink(symlink2, symlink3);
        IndexerRunner runner(shared_ptr<Source>(new FileSource(symlink3)), _indexFile(&res), BlockDistanceStorageDecisionStrategy::getDefault());
        bool result = runner.fulfillsPremises();
                CHECK_EQUAL(true, result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN);
        path fastq = sourceFile(&res);
        path symlink1 = path(fastq.string() + ".link1");
        create_symlink(fastq, symlink1);
        IndexerRunner runner(shared_ptr<Source>(new FileSource(symlink1)), _indexFile(&res), BlockDistanceStorageDecisionStrategy::getDefault());
        bool result = runner.fulfillsPremises();
                CHECK_EQUAL(false, result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_EXISTING_INDEX) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_EXISTING_INDEX);
        res.createEmptyFile(FASTQ_FILENAME);
        res.createEmptyFile(INDEX_FILENAME);
        IndexerRunner runner(_sourceFile(&res), _indexFile(&res), BlockDistanceStorageDecisionStrategy::getDefault());
        bool result = runner.fulfillsPremises();
        // It is not allowed to override an existing file! Test has to "fail"
                CHECK(!result);
    }
}