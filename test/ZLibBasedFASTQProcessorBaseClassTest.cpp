/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/ZLibBasedFASTQProcessorBaseClass.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>
#include <zlib.h>

const char *const TEST_ZLIBBASE_SUITE = "Test suite for ZLibBasedFASTQProcessorBaseClass tests.";
const char *const TEST_ZLIBBASE_CREATION = "Test creation of ZLibBasedFASTQProcessorBaseClass with a mock class.";

class ZLibBasedFASTQProcessorBaseTestClass : public ZLibBasedFASTQProcessorBaseClass {
public:
    ZLibBasedFASTQProcessorBaseTestClass(
            const path &fastq,
            const path &index)
            : ZLibBasedFASTQProcessorBaseClass(fastq, index, true) {}

    z_stream *getZStream() {
        return &zStream;
    }
};

SUITE (TEST_ZLIBBASE_SUITE) {
    TEST (TEST_ZLIBBASE_CREATION) {
        TestResourcesAndFunctions res(TEST_ZLIBBASE_SUITE, TEST_ZLIBBASE_CREATION);

        path fastq = res.getResource(string("test2.fastq.gz"));
        path index = res.filePath("test2.fastq.gz.idx");

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
}
