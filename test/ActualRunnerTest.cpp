/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/ActualRunner.h"
#include "../src/IndexerRunner.h"
#include "../src/ErrorMessages.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>
#include <experimental/filesystem>

using std::experimental::filesystem::path;

const char *TEST_FILEPATH_HELPER_METHOD = "testFilePathHelperMethod";
const char *TEST_CHECK_PREMISES_WITH_FASTQ = "testCheckPremisesWithFastq";
const char *TEST_CHECK_PREMISES_WITH_MISSING_FASTQ = "testCheckPremisesWithMissingFastq";
const char *TEST_CHECK_PREMISES_WITH_EXISTING_INDEX = "testCheckPremisesWithExistingIndex";
const char *TEST_CHECK_PREMISES_WITH_FASTQ_BEHIND_SYMLINK_CHAIN = "testCheckPremisesWithFastqBehindSymlinkChain";
const char *TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN = "testCheckPremisesWithUnreadbleFastqBehindSymlinkChain";

const char *SUITE_ID = "RunnerTests";

path fastqFile(TestResourcesAndFunctions *res) {
    return res->filePath(FASTQ_FILENAME);
}

path indexFile(TestResourcesAndFunctions *res) {
    return res->filePath(INDEX_FILENAME);
}

SUITE (SUITE_ID) {

    TEST (TEST_FILEPATH_HELPER_METHOD) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_FILEPATH_HELPER_METHOD);
                CHECK(fastqFile(&res) == res.getTestPath().string() + "/" + FASTQ_FILENAME);
                CHECK(indexFile(&res) == res.getTestPath().string() + "/" + INDEX_FILENAME);
    }

    TEST (TEST_CHECK_PREMISES_WITH_MISSING_FASTQ) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_MISSING_FASTQ);
        IndexerRunner *runner = new IndexerRunner(fastqFile(&res), indexFile(&res), -1);
        bool result = runner->checkPremises();
                CHECK(!result);
//                CHECK(runner->getErrorMessages().size() == 1);
//                CHECK(runner->getErrorMessages()[0] == ERR_MESSAGE_FASTQ_INVALID);
        delete runner;
    }

    TEST (TEST_CHECK_PREMISES_WITH_FASTQ) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_FASTQ);
        path fastq = fastqFile(&res);
        res.createEmptyFile(FASTQ_FILENAME);
        IndexerRunner runner(fastq, indexFile(&res));
        bool result = runner.checkPremises();
                CHECK(result);
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
        IndexerRunner runner(symlink3, indexFile(&res));
        bool result = runner.checkPremises();
                CHECK_EQUAL(true, result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_UNREADABLE_FASTQ_BEHIND_SYMLINK_CHAIN);
        path fastq = fastqFile(&res);
        path symlink1 = path(fastq.string() + ".link1");
        create_symlink(fastq, symlink1);
        IndexerRunner runner(symlink1, indexFile(&res));
        bool result = runner.checkPremises();
                CHECK_EQUAL(false, result);
    }

    TEST (TEST_CHECK_PREMISES_WITH_EXISTING_INDEX) {
        TestResourcesAndFunctions res(SUITE_ID, TEST_CHECK_PREMISES_WITH_EXISTING_INDEX);
        res.createEmptyFile(FASTQ_FILENAME);
        res.createEmptyFile(INDEX_FILENAME);
        IndexerRunner runner(fastqFile(&res), indexFile(&res));
        bool result = runner.checkPremises();
        // It is not allowed to override an existing file! Test has to "fail"
                CHECK(!result);
    }
}