/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/Source.h"
#include "process/io/StreamSource.h"
#include "process/io/FileSource.h"
#include "process/io/s3/S3Source.h"
#include "process/io/Sink.h"
#include "process/io/FileSink.h"
#include "process/io/s3/S3Sink.h"
#include "startup/ModeCLIParser.h"
#include <UnitTest++/UnitTest++.h>

const char *const SUITE_MODECLIPARSER_TESTS = "Tests for the ModeCLIParser class";
const char *const TEST_RESOLVE_INDEX_FILENAME = "Test ::resolveIndexFileName()";
const char *const TEST_PROCESS_FASTQ_FILE_NONE = "Test ::processSourceFileSource() with invalid FileSource result";
const char *const TEST_PROCESS_FASTQ_FILE_S3 = "Test ::processSourceFileSource() with S3Source result";
const char *const TEST_PROCESS_FASTQ_FILE_STREAM = "Test ::processSourceFileSource() with StreamSource result";
const char *const TEST_PROCESS_FASTQ_FILE_PATH = "Test ::processSourceFileSource() with FileSource result";
const char *const TEST_PROCESS_INDEX_FILE_SOURCE_PATH = "Test ::processIndexFileSource() with FileSource";
const char *const TEST_PROCESS_INDEX_FILE_SOURCE_S3 = "Test ::processIndexFileSource() with S3Source";
const char *const TEST_PROCESS_INDEX_FILE_SINK_PATH = "Test ::processIndexFileSink() with FileSink result";
const char *const TEST_PROCESS_INDEX_FILE_SINK_PATH_NONAME = "Test ::processIndexFileSink() without a filename and with FileSink result";
const char *const TEST_PROCESS_INDEX_FILE_SINK_S3 = "Test ::processIndexFileSink() with S3Sink result";
const char *const TEST_PROCESS_INDEX_FILE_SINK_S3_NONAME = "Test ::processIndexFileSink() without a filename and with S3Sink result";
const char *const TEST_PROCESS_FILE_SINK = "Test ::processFileSink()";
//const char *const TEST_TESTS3SERVICECOMPATIBILITY = "Test TEST_TESTS3SERVICECOMPATIBILITY";

S3Service_S getTestService() {
    return S3Service::from(S3ServiceOptions(string(""), "", ""));
}

SUITE (SUITE_MODECLIPARSER_TESTS) {
    TEST (TEST_RESOLVE_INDEX_FILENAME) {
        auto fq = FileSource::from("abc.fastq");
        // Needs full path because the name is inherited from the FASTQ FileSource (which is fully resolved)
                CHECK_EQUAL(IOHelper::fullPath("abc.fastq.fqi"), ModeCLIParser::resolveIndexFileName("", fq));
        // Short check here
                CHECK_EQUAL("a.fqi", ModeCLIParser::resolveIndexFileName("a.fqi", fq));
        auto fqs3 = S3Source::from("s3://abc/abc.fastq", getTestService());
                CHECK_EQUAL(string("s3://abc/abc.fastq.fqi"), ModeCLIParser::resolveIndexFileName("", fqs3));
    }

    TEST (TEST_PROCESS_FASTQ_FILE_NONE) {
        // This one is an invalid, unusable FileSource
        auto src = ModeCLIParser::processSourceFileSource("", getTestService());
                CHECK(dynamic_pointer_cast<FileSource>(src)->toString() == "");
    }

    TEST (TEST_PROCESS_FASTQ_FILE_PATH) {
        // This one is an invalid, unusable FileSource
        auto src = ModeCLIParser::processSourceFileSource("abc.fastq", getTestService());
                CHECK(dynamic_pointer_cast<FileSource>(src)->toString() == IOHelper::fullPath("abc.fastq"));
    }

    TEST (TEST_PROCESS_FASTQ_FILE_S3) {
        // This one is an invalid, unusable FileSource
        auto src = ModeCLIParser::processSourceFileSource("s3://abc/test.fastq", getTestService());
                CHECK(dynamic_pointer_cast<S3Source>(src)->toString() == "s3://abc/test.fastq");
    }

    TEST (TEST_PROCESS_FASTQ_FILE_STREAM) {
        // This one is an invalid, unusable FileSource
        auto src = ModeCLIParser::processSourceFileSource("-", getTestService());
                CHECK(dynamic_pointer_cast<StreamSource>(src)->getStream() == &std::cin);
    }

    TEST (TEST_PROCESS_INDEX_FILE_SOURCE_PATH) {
        // Index filename resolve is done in a previous step.
        auto src = ModeCLIParser::processIndexFileSource("test.fqi", getTestService());
                CHECK(dynamic_pointer_cast<FileSource>(src)->toString() == IOHelper::fullPath("test.fqi"));
    }

    TEST (TEST_PROCESS_INDEX_FILE_SOURCE_S3) {
        // Index filename resolve is done in a previous step.
        auto src = ModeCLIParser::processIndexFileSource("s3://abc/test.fqi", getTestService());
                CHECK(dynamic_pointer_cast<S3Source>(src)->toString() == "s3://abc/test.fqi");
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_PATH) {
        auto src = ModeCLIParser::processIndexFileSink(
                "test.fqi",
                true,
                FileSource::from("test.fastq"),
                getTestService()
        );
                CHECK(dynamic_pointer_cast<FileSink>(src)->toString() == IOHelper::fullPath("test.fqi"));
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_PATH_NONAME) {
        auto src = ModeCLIParser::processIndexFileSink(
                "",
                true,
                FileSource::from("test.fastq"),
                getTestService()
        );
                CHECK(dynamic_pointer_cast<FileSink>(src)->toString() == IOHelper::fullPath("test.fastq.fqi"));
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_S3) {
        auto service = getTestService();
        auto src = ModeCLIParser::processIndexFileSink(
                "s3://abc/test.fqi",
                true,
                S3Source::from("s3://abc/test.fastq", service),
                service
        );
                CHECK(dynamic_pointer_cast<S3Sink>(src)->toString() == "s3://abc/test.fqi");
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_S3_NONAME) {
        auto service = getTestService();
        auto src = ModeCLIParser::processIndexFileSink(
                "",
                true,
                S3Source::from("s3://abc/test.fastq", service),
                service
        );
                CHECK(dynamic_pointer_cast<S3Sink>(src)->toString() == "s3://abc/test.fastq.fqi");
    }

//    TEST (TEST_TESTS3SERVICECOMPATIBILITY) {
//
//    }
}