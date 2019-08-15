/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/Source.h"
#include "common/IOHelper.h"
#include "process/io/FileSource.h"
#include "process/base/ZLibBasedFASTQProcessorBaseClass.h"
#include "../../TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>
#include <zlib.h>

const char *const TEST_ZLIBBASE_SUITE = "Test suite for ZLibBasedFASTQProcessorBaseClass tests.";
const char *const TEST_ZLIBBASE_CREATION = "Test creation of ZLibBasedFASTQProcessorBaseClass with a mock class.";

class ZLibBasedFASTQProcessorBaseTestClass : public ZLibBasedFASTQProcessorBaseClass {
public:
    ZLibBasedFASTQProcessorBaseTestClass(
            const path &fastq,
            const path &index)
            : ZLibBasedFASTQProcessorBaseClass(shared_ptr<Source>(new FileSource(fastq)), make_shared<FileSource>(index), true) {}

    z_stream *getZStream() {
        return &zStream;
    }
};

SUITE (TEST_ZLIBBASE_SUITE) {
    TEST (TEST_ZLIBBASE_CREATION) {
        TestResourcesAndFunctions res(TEST_ZLIBBASE_SUITE, TEST_ZLIBBASE_CREATION);

        path fastq = TestResourcesAndFunctions::getResource(string(TEST_FASTQ_LARGE));
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
}
