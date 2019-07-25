/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/Source.h"
#include "process/io/StreamSource.h"
#include "process/io/PathSource.h"
#include "process/io/s3/S3Source.h"
#include "process/io/Sink.h"
#include "process/io/PathSink.h"
#include "process/io/s3/S3Sink.h"
#include "startup/ModeCLIParser.h"
#include <UnitTest++/UnitTest++.h>

const char *const SUITE_MODECLIPARSER_TESTS = "Tests for the ModeCLIParser class";
const char *const TEST_RESOLVE_INDEX_FILENAME = "Test ::resolveIndexFileName()";
const char *const TEST_PROCESS_FASTQ_FILE_NONE = "Test ::processFastqFile() with invalid PathSource result";
const char *const TEST_PROCESS_FASTQ_FILE_S3 = "Test ::processFastqFile() with S3Source result";
const char *const TEST_PROCESS_FASTQ_FILE_STREAM = "Test ::processFastqFile() with StreamSource result";
const char *const TEST_PROCESS_FASTQ_FILE_PATH = "Test ::processFastqFile() with PathSource result";
const char *const TEST_PROCESS_INDEX_FILE_SOURCE_PATH = "Test ::processIndexFileSource() with PathSource";
const char *const TEST_PROCESS_INDEX_FILE_SOURCE_S3 = "Test ::processIndexFileSource() with S3Source";
const char *const TEST_PROCESS_INDEX_FILE_SINK_PATH = "Test ::processIndexFileSink() with PathSink result";
const char *const TEST_PROCESS_INDEX_FILE_SINK_PATH_NONAME = "Test ::processIndexFileSink() without a filename and with PathSink result";
const char *const TEST_PROCESS_INDEX_FILE_SINK_S3 = "Test ::processIndexFileSink() with S3Sink result";
const char *const TEST_PROCESS_INDEX_FILE_SINK_S3_NONAME = "Test ::processIndexFileSink() without a filename and with S3Sink result";
const char *const TEST_PROCESS_FILE_SINK = "Test ::processFileSink()";

SUITE (SUITE_MODECLIPARSER_TESTS) {
    TEST (TEST_RESOLVE_INDEX_FILENAME) {
        auto fq = PathSource::from("abc.fastq");
        // Needs full path because the name is inherited from the FASTQ PathSource (which is fully resolved)
                CHECK_EQUAL(IOHelper::fullPath("abc.fastq.fqi"), ModeCLIParser::resolveIndexFileName("", fq));
        // Short check here
                CHECK_EQUAL("a.fqi", ModeCLIParser::resolveIndexFileName("a.fqi", fq));
        auto fqs3 = S3Source::from("s3://abc/abc.fastq", S3ServiceOptions(string(""), "", ""));
                CHECK_EQUAL(string("s3://abc/abc.fastq.fqi"), ModeCLIParser::resolveIndexFileName("", fqs3));
    }

    TEST (TEST_PROCESS_FASTQ_FILE_NONE) {
        // This one is an invalid, unusable PathSource
        auto src = ModeCLIParser::processFastqFile("", S3ServiceOptions("", "", ""));
                CHECK(dynamic_pointer_cast<PathSource>(src)->toString() == "");
    }

    TEST (TEST_PROCESS_FASTQ_FILE_PATH) {
        // This one is an invalid, unusable PathSource
        auto src = ModeCLIParser::processFastqFile("abc.fastq", S3ServiceOptions("", "", ""));
                CHECK(dynamic_pointer_cast<PathSource>(src)->toString() == IOHelper::fullPath("abc.fastq"));
    }

    TEST (TEST_PROCESS_FASTQ_FILE_S3) {
        // This one is an invalid, unusable PathSource
        auto src = ModeCLIParser::processFastqFile("s3://abc/test.fastq", S3ServiceOptions("", "", ""));
                CHECK(dynamic_pointer_cast<S3Source>(src)->toString() == "s3://abc/test.fastq");
    }

    TEST (TEST_PROCESS_FASTQ_FILE_STREAM) {
        // This one is an invalid, unusable PathSource
        auto src = ModeCLIParser::processFastqFile("-", S3ServiceOptions("", "", ""));
                CHECK(dynamic_pointer_cast<StreamSource>(src)->getStream() == &std::cin);
    }

    TEST (TEST_PROCESS_INDEX_FILE_SOURCE_PATH) {
        // Index filename resolve is done in a previous step.
        auto src = ModeCLIParser::processIndexFileSource("test.fqi",S3ServiceOptions( "", "", ""));
                CHECK(dynamic_pointer_cast<PathSource>(src)->toString() == IOHelper::fullPath("test.fqi"));
    }

    TEST (TEST_PROCESS_INDEX_FILE_SOURCE_S3) {
        // Index filename resolve is done in a previous step.
        auto src = ModeCLIParser::processIndexFileSource("s3://abc/test.fqi", S3ServiceOptions("", "", ""));
                CHECK(dynamic_pointer_cast<S3Source>(src)->toString() == "s3://abc/test.fqi");
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_PATH) {
        auto src = ModeCLIParser::processIndexFileSink("test.fqi", true, PathSource::from("test.fastq"), S3ServiceOptions("", "", ""));
                CHECK(dynamic_pointer_cast<PathSink>(src)->toString() == IOHelper::fullPath("test.fqi"));
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_PATH_NONAME) {
        auto src = ModeCLIParser::processIndexFileSink("", true, PathSource::from("test.fastq"), S3ServiceOptions("", "", ""));
                CHECK(dynamic_pointer_cast<PathSink>(src)->toString() == IOHelper::fullPath("test.fastq.fqi"));
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_S3) {
        auto src = ModeCLIParser::processIndexFileSink("s3://abc/test.fqi", true,
                                                       S3Source::from("s3://abc/test.fastq", S3ServiceOptions(string(""), "", "")), S3ServiceOptions("",
                                                       "", ""));
                CHECK(dynamic_pointer_cast<S3Sink>(src)->toString() == "s3://abc/test.fqi");
    }

    TEST (TEST_PROCESS_INDEX_FILE_SINK_S3_NONAME) {
        auto src = ModeCLIParser::processIndexFileSink("", true,
                                                       S3Source::from("s3://abc/test.fastq", S3ServiceOptions(string(""), "", "")), S3ServiceOptions("",
                                                       "", ""));
                CHECK(dynamic_pointer_cast<S3Sink>(src)->toString() == "s3://abc/test.fastq.fqi");
    }

}