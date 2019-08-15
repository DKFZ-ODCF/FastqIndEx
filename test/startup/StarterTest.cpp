/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/ConsoleSink.h"
#include "process/io/FileSource.h"
#include "process/io/FileSink.h"
#include "runners/ExtractorRunner.h"
#include "runners/IndexerRunner.h"
#include "startup/Starter.h"
#include "TestConstants.h"

#include <cstring>
#include <experimental/filesystem>
#include <UnitTest++/UnitTest++.h>

using std::experimental::filesystem::path;

using namespace std;

const char *const TEST_STATSMODE_RUNNER_CREATE_VALIDPARMS = "Create StatsModeRunner with valid parameters";
const char *const TEST_CREATE_INDEXRUNNER_VALIDPARMS_WINDEX = "Test create IndexRunner with valid parameters with index";
const char *const TEST_CREATE_INDEXRUNNER_VALIDPARMS_WOINDEX = "Test create IndexRunner with valid parameters without index";
const char *const TEST_CREATE_EXTRACTORRUNNER_VALIDPARMS_WINDEX = "Test create ExtractorRunner with valid parameters with index";
const char *const TEST_CREATE_EXTRACTORRUNNER_VALIDPARMS_WOINDEX = "Test create ExtractorRunner with valid parameters without index";

SUITE (StarterTests) {
    TEST (testCreateNewRunners) {
        auto sourceFile = shared_ptr<Source>(new FileSource("/tmp/abc"));
        path indexFile("/tmp/abc.fqi");
        IndexerRunner _runner(sourceFile, make_shared<FileSink>(indexFile),
                              BlockDistanceStorageDecisionStrategy::from(1));
                CHECK_EQUAL(dynamic_pointer_cast<FileSource>(_runner.getSourceFile())->getPath(),
                            dynamic_pointer_cast<FileSource>(sourceFile)->getPath());
                CHECK_EQUAL(_runner.getIndexFile()->toString(), indexFile.string());
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
        //   - sourceFile
        //   - indexFile (optional)
        //   in case of extract
        //   - sourceFile
        //   - indexFile (optional)
        //   - readstart / firstline
        //   - readend / lastline (or amount?)

//        Starter starter;
//        const char *argv[] = {"wrong"};
//
//        auto _runner = starter.createRunner(1, argv);
//
//        // Strings in the array cannot be deleted, but delete the array itself immediately.
//        CHECK(_runner && _runner->isCLIOptionsPrinter());
//
//        auto runner = static_pointer_cast<DoNothingRunner>(_runner);

    }

    TEST (TEST_CREATE_INDEXRUNNER_VALIDPARMS_WINDEX) {
        const char *argv[] = {TEST_BINARY, "index", "-f=afastq.gz", "-i=afastq.fqi"};

        Starter starter;
        auto _runner = starter.createRunner(4, argv);
                CHECK (_runner && _runner->isIndexer());

        auto runner = static_pointer_cast<IndexerRunner>(_runner);
        auto fastq = dynamic_pointer_cast<FileSource>(runner->getSourceFile());
        auto index = dynamic_pointer_cast<FileSink>(runner->getIndexFile());
                CHECK(fastq.get());
                CHECK(index.get());
                CHECK_EQUAL(IOHelper::fullPath("afastq.gz"), fastq->toString());
                CHECK_EQUAL(IOHelper::fullPath("afastq.fqi"), index->toString());
    }

    TEST (TEST_CREATE_INDEXRUNNER_VALIDPARMS_WOINDEX) {

        const char *argv[] = {TEST_BINARY, "index", "-f=afastq.gz"};

        Starter starter;
        auto _runner = starter.createRunner(3, argv);
                CHECK (_runner && _runner->isIndexer());

        auto runner = static_pointer_cast<IndexerRunner>(_runner);
        auto fastq = dynamic_pointer_cast<FileSource>(runner->getSourceFile());
        auto index = dynamic_pointer_cast<FileSink>(runner->getIndexFile());
                CHECK(fastq.get());
                CHECK(index.get());
                CHECK_EQUAL(IOHelper::fullPath("afastq.gz"), fastq->toString());
                CHECK_EQUAL(IOHelper::fullPath("afastq.gz.fqi"), index->toString());
    }

    TEST (TEST_CREATE_EXTRACTORRUNNER_VALIDPARMS_WINDEX) {

        Starter starter;
        const char *argv[] = {
                TEST_BINARY,
                "extract",
                "-f=test2.fastq.gz",
                "-i=fastq.gz.fqi",
                "-o=-",
                "-s=0",
                "-n=10"
        };

        auto _runner = starter.createRunner(7, argv);
                CHECK (_runner && _runner->isExtractor());

        // Use dynamic casts to make sure, that the proper classes are generated.
        auto runner = static_pointer_cast<ExtractorRunner>(_runner);
        auto fastq = dynamic_pointer_cast<FileSource>(runner->getSourceFile());
        auto index = dynamic_pointer_cast<FileSource>(runner->getIndexFile());
        auto result = dynamic_pointer_cast<ConsoleSink>(runner->getResultSink());
                CHECK(fastq.get());
                CHECK(index.get());
                CHECK(result.get());
                CHECK_EQUAL(IOHelper::fullPath("test2.fastq.gz"), fastq->toString());
                CHECK_EQUAL(IOHelper::fullPath("fastq.gz.fqi"), index->toString());
                CHECK_EQUAL("cout", result->toString());
    }

    TEST (TEST_CREATE_EXTRACTORRUNNER_VALIDPARMS_WOINDEX) {
        Starter starter;
        const char *argv[] = {
                TEST_BINARY,
                "extract",
                "-f=test2.fastq.gz",
                "-o=result.out",
                "-s=0",
                "-n=10"
        };

        auto _runner = starter.createRunner(6, argv);

                CHECK (_runner && _runner->isExtractor());

        // Use dynamic casts to make sure, that the proper classes are generated.
        auto runner = static_pointer_cast<ExtractorRunner>(_runner);
        auto fastq = dynamic_pointer_cast<FileSource>(runner->getSourceFile());
        auto index = dynamic_pointer_cast<FileSource>(runner->getIndexFile());
        auto result = dynamic_pointer_cast<FileSink>(runner->getResultSink());
                CHECK(fastq.get());
                CHECK(index.get());
                CHECK(result.get());
                CHECK_EQUAL(IOHelper::fullPath("test2.fastq.gz"), fastq->toString());
                CHECK_EQUAL(IOHelper::fullPath("test2.fastq.gz.fqi"), index->toString());
                CHECK_EQUAL(IOHelper::fullPath("result.out"), result->toString());
    }

    TEST (TEST_STATSMODE_RUNNER_CREATE_VALIDPARMS) {
        const char *argv[] = {TEST_BINARY, "stats", "-i=test2.fastq.gz.fqi",};

        Starter starter;
        auto _runner = starter.createRunner(3, argv);
        auto runner = dynamic_pointer_cast<IndexStatsRunner>(_runner);
        auto index = dynamic_pointer_cast<FileSource>(runner->getIndexFile());
                CHECK(index.get());
                CHECK(index->toString() == IOHelper::fullPath("test2.fastq.gz.fqi"));
    }
}