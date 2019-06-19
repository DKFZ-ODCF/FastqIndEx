/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/process/io/InputSource.h"
#include "../src/process/io/PathInputSource.h"
#include "../src/process/base/ZLibBasedFASTQProcessorBaseClass.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>
#include <zlib.h>

const char *const TEST_ZLIBBASE_SUITE = "Test suite for ZLibBasedFASTQProcessorBaseClass tests.";
const char *const TEST_ZLIBBASE_CREATION = "Test creation of ZLibBasedFASTQProcessorBaseClass with a mock class.";
const char *const TEST_SPLIT_STR = "Test splitStr()";

class ZLibBasedFASTQProcessorBaseTestClass : public ZLibBasedFASTQProcessorBaseClass {
public:
    ZLibBasedFASTQProcessorBaseTestClass(
            const path &fastq,
            const path &index)
            : ZLibBasedFASTQProcessorBaseClass(shared_ptr<InputSource>(new PathInputSource(fastq)), index, true) {}

    z_stream *getZStream() {
        return &zStream;
    }
};

SUITE (TEST_ZLIBBASE_SUITE) {
    TEST (TEST_ZLIBBASE_CREATION) {
        TestResourcesAndFunctions res(TEST_ZLIBBASE_SUITE, TEST_ZLIBBASE_CREATION);

        path fastq = res.getResource(string(TEST_FASTQ_LARGE));
        path index = res.filePath("test2.fastq.gz.fqi");

        ZLibBasedFASTQProcessorBaseTestClass mock(fastq, index);
        z_stream *zStream = mock.getZStream();
        bool ok = mock.initializeZStreamForInflate();
                CHECK(ok);
                CHECK(zStream->zalloc != nullptr);
                CHECK(zStream->zfree != nullptr);    // Will be filled by inflateInit2
                CHECK(zStream->opaque == nullptr);   // Will be filled by inflateInit2
                CHECK(zStream->next_in == nullptr);
                CHECK(zStream->avail_in == 0);
    }

    TEST (TEST_SPLIT_STR) {
        auto text = string("one\ntwo\nthree\nfour\n5\n6\n7\n\n");
        vector<string> expectedVector{
                "one", "two", "three", "four", "5", "6", "7", ""
        };
        vector<string> res = ZLibBasedFASTQProcessorBaseClass::splitStr(text);
                CHECK_EQUAL(expectedVector.size(), res.size());
                CHECK_ARRAY_EQUAL(expectedVector, res, res.size());
    }
}
