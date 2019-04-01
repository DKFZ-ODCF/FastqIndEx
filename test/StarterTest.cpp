/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/Starter.h"
#include "../src/IndexerRunner.h"
#include "../src/ExtractorRunner.h"
#include "TestConstants.h"

#include <cstring>
#include <UnitTest++/UnitTest++.h>
#include <experimental/filesystem>

using std::experimental::filesystem::path;

using namespace std;

SUITE (StarterTests) {
    TEST (testCreateNewRunners) {
        path fastqFile("/tmp/abc");
        path indexFile("/tmp/abc.idx");
        IndexerRunner runner(fastqFile, indexFile);
                CHECK_EQUAL(runner.getFastqFile(), fastqFile);
                CHECK_EQUAL(runner.getIndexFile(), indexFile);
    }

    TEST (testCreateRunnerWithInvalidParameters) {

        // We need to validate:
        //   mode (index / extract)
        //   in case of index
        //   - fastqFile
        //   - indexFile (optional)
        //   in case of extract
        //   - fastqFile
        //   - indexFile (optional)
        //   - readstart / firstline
        //   - readend / lastline (or amount?)

        Starter starter;
        const char *argv[] = {"wrong"};

        auto runner = starter.createRunner(1, argv);

        // Strings in the array cannot be deleted, but delete the array itself immediately.
        CHECK(runner && runner->isCLIOptionsPrinter());

        auto explicitRunner = static_pointer_cast<DoNothingRunner>(runner);

    }

    TEST (testCreateRunnerWithValidIndexParametersWithoutIndex) {

        // We need to validate:
        //   mode (index / extract)
        //   in case of index
        //   - fastqFile
        //   - indexFile (optional)

        Starter starter;
        const char *argv[] = {
                TEST_BINARY,
                "--index",
                "afastq.gz"
        };

        auto runner = starter.createRunner(3, argv);

                CHECK (runner && runner->isIndexer());

        auto explicitRunner = static_pointer_cast<IndexerRunner>(runner);
    }

    TEST (testCreateRunnerWithValidIndexParametersWithIndex) {

        // We need to validate:
        //   mode (index / extract)
        //   in case of index
        //   - fastqFile
        //   - indexFile (optional)

        Starter starter;
        const char *argv[] = {
                TEST_BINARY,
                "--index",
                "test2.fastq.gz"
        };

        auto runner = starter.createRunner(3, argv);

                CHECK (runner && runner->isIndexer());

        auto explicitRunner = static_pointer_cast<IndexerRunner>(runner);
    }

    TEST (testCreateRunnerWithValidExtractParameters) {

        // We need to validate:
        //   mode (index / extract)
        //   in case of extract
        //   - fastqFile
        //   - indexFile (optional)
        //   - readstart / firstline
        //   - readend / lastline (or amount?)

        Starter starter;
        const char *argv[] = {
                TEST_BINARY,
                "--extract",
                "test2.fastq.gz",
                "--startline",
                "0",
                "--noofreads",
                "10"
        };

        // TODO Check with wrong parameters.

        auto runner = starter.createRunner(7, argv);

                CHECK (runner && runner->isExtractor());

        auto explicitRunner = static_pointer_cast<ExtractorRunner>(runner);
    }
}