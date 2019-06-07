/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/ExtractorRunner.h"
#include "../src/IndexerRunner.h"
#include "../src/Starter.h"
#include "../src/PathInputSource.h"
#include "TestConstants.h"

#include <cstring>
#include <experimental/filesystem>
#include <UnitTest++/UnitTest++.h>

using std::experimental::filesystem::path;

using namespace std;

SUITE (StarterTests) {
    TEST (testCreateNewRunners) {
        auto fastqFile = shared_ptr<InputSource>(new PathInputSource("/tmp/abc"));
        path indexFile("/tmp/abc.fqi");
        IndexerRunner runner(fastqFile, indexFile);
                CHECK_EQUAL(dynamic_pointer_cast<PathInputSource>(runner.getFastqFile())->getPath(), dynamic_pointer_cast<PathInputSource>(fastqFile)->getPath());
                CHECK_EQUAL(runner.getIndexFile(), indexFile);
    }

    /**
     * We need to disable this test. TCLAP seems to call exit() when errors pop up. I found the -Dexit=method flag to
     * allow the compiler to run with a custom exit method but I could not get it running. Leaving it like it is, the
     * test will not only fail but will also crash the whole application. So for now, TCLAP tests must work and must not
     * test for bad input.
     */
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

//        Starter starter;
//        const char *argv[] = {"wrong"};
//
//        auto runner = starter.createRunner(1, argv);
//
//        // Strings in the array cannot be deleted, but delete the array itself immediately.
//        CHECK(runner && runner->isCLIOptionsPrinter());
//
//        auto explicitRunner = static_pointer_cast<DoNothingRunner>(runner);

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
                "index",
                "-f=afastq.gz"
        };

        auto runner = starter.createRunner(3, argv);

                CHECK (runner && runner->isIndexer());

        auto explicitRunner = static_pointer_cast<IndexerRunner>(runner);
                CHECK(explicitRunner->getIndexFile().filename() == "afastq.gz.fqi");
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
                "index",
                "-f=afastq.gz",
                "-i=afastq.fqi"
        };

        auto runner = starter.createRunner(4, argv);

                CHECK (runner && runner->isIndexer());

        auto explicitRunner = static_pointer_cast<IndexerRunner>(runner);
                CHECK(explicitRunner->getIndexFile().filename() == "afastq.fqi");
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
                "extract",
                "-f=test2.fastq.gz",
                "-i=test2.fastq.gz.fqi",
                "-o=-",
                "-s=0",
                "-n=10"
        };

        // TODO Check with wrong parameters.

        auto runner = starter.createRunner(7, argv);

                CHECK (runner && runner->isExtractor());

        auto explicitRunner = static_pointer_cast<ExtractorRunner>(runner);
//        CHECK_EQUAL(explicitRunner->getIndexFile() )
    }
}