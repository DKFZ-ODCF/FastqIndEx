/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

const char *const S3_SINK_TEST_SUITE = "Test suite for the S3Sink class";

#include "process/io/s3/S3Sink.h"
#include "process/io/s3/FQIS3TestClient.h"
#include "process/io/FileSource.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

SUITE (S3_SINK_TEST_SUITE) {

    const char *const S3_SINK_CONSTRUCT = "Test construction and fulfillsPremises()";
    const char *const S3_SINK_OPEN = "Test open and close";
    const char *const S3_SINK_OPENLOCKED = "Test open and close with lock_unlock";
    const char *const S3_SINK_AQUIRELOCK_LATER = "Test aquire lock after file open";
    const char *const S3_SINK_WRITE_TELL_SEEK = "Test write tell seek rewind functions";
    const char *const S3_SINK_WRITE_OVERWRITEBYTES = "Test write rewind_seek overwrite bytes";

    shared_ptr<S3Sink> createTestSinkWithBackingFile(const path &file) {
        auto client = FQIS3TestClient::from(
                TestResourcesAndFunctions::getResource(file),
                S3Service::getDefault()
        );
        return S3Sink::from(client, true);
    }

    shared_ptr<S3Sink> createTestSink() {
        return createTestSinkWithBackingFile(TEST_FASTQ_SMALL);
    }

    const char *const S3_PATH("s3://TestBucket/test.fastq.gz");

    TEST (S3_SINK_CONSTRUCT) {
        TestResourcesAndFunctions res(S3_SINK_TEST_SUITE, S3_SINK_CONSTRUCT);

        auto p = createTestSinkWithBackingFile(TEST_FASTQ_SMALL);
                CHECK(p->fulfillsPremises());
                CHECK(!p->exists());
                CHECK(p->hasLock());  // Always true for S3 now
                CHECK(!p->isOpen());
                CHECK(p->isGood());
                CHECK(!p->isBad());
                CHECK(!p->eof());
                CHECK(p->isFile());
                CHECK(p->isStream());
                CHECK(p->toString() == S3_PATH);
                CHECK_EQUAL(string(S3_PATH), p->toString());
                CHECK_EQUAL(-1, p->size());
                CHECK_EQUAL(-1, p->tell());
                CHECK(p->getErrorMessages().empty());
                CHECK(!p->canRead());
                CHECK(!p->canWrite()); // Not open yet

//        auto p2 = S3Sink(res.createEmptyFile("noOverwrite"));
//                CHECK(p2.exists());
//                CHECK(!p2.fulfillsPremises());
//
//        auto p3 = S3Sink(res.createEmptyFile("overwrite"), true);
//                CHECK(p3.exists());
//                CHECK(p3.fulfillsPremises());
    }


    TEST (S3_SINK_OPEN) {
        TestResourcesAndFunctions res(S3_SINK_TEST_SUITE, S3_SINK_OPEN);

        auto p = createTestSink();

        p->open();
                CHECK(p->isOpen());
        p->close();
                CHECK(!p->isOpen());
    }

    TEST (S3_SINK_OPENLOCKED) {
        TestResourcesAndFunctions res(S3_SINK_TEST_SUITE, S3_SINK_OPENLOCKED);
        auto p = createTestSink();
                CHECK(!p->isOpen());

        p->openWithWriteLock();

                CHECK(p->isOpen());
                CHECK(p->hasLock());

        p->close();
                CHECK(!p->isOpen());
                CHECK(p->hasLock());
    }

    TEST (S3_SINK_AQUIRELOCK_LATER) {
        TestResourcesAndFunctions res(S3_SINK_TEST_SUITE, S3_SINK_AQUIRELOCK_LATER);

        auto p = createTestSink();
                CHECK(!p->isOpen());

        p->open();
                CHECK(p->hasLock());
                CHECK(p->isOpen());

        p->openWithWriteLock();
                CHECK(p->hasLock());
                CHECK(p->isOpen());
    }

    TEST (S3_SINK_WRITE_TELL_SEEK) {
        TestResourcesAndFunctions res(S3_SINK_TEST_SUITE, S3_SINK_WRITE_TELL_SEEK);

        auto p = createTestSink();
        p->openWithWriteLock();
                CHECK(p->isOpen());

        string test1("AShortMessage");
        p->write(test1);

                CHECK(p->tell() == static_cast<int64_t>(test1.length()));
        p->seek(0, true);
                CHECK(p->tell() == 0);
        p->seek(test1.length(), false);
                CHECK(p->tell() == static_cast<int64_t>(test1.length()));
        p->rewind(2);
                CHECK(p->tell() == static_cast<int64_t>(test1.length() - 2));

        p->seek(p->size() + 1, true);
                CHECK(p->tell() ==
                      static_cast<int64_t>(test1.length() + 1)); // This is only working, because of our opening mode!
//                CHECK(p->eof());                            // This is not working, because of our opening mode!

        const char *c_str = test1.c_str();
        p->write(c_str, 4);
                CHECK(p->tell() == static_cast<int64_t>(test1.length() + 5));

        p->write(c_str);
                CHECK(p->tell() == static_cast<int64_t>(2 * test1.length() + 5));
    }

//    TEST (S3_SINK_WRITE_OVERWRITEBYTES) {
//        TestResourcesAndFunctions res(S3_SINK_TEST_SUITE, S3_SINK_WRITE_OVERWRITEBYTES);
//
//        auto file = res.createEmptyFile("abc.txt");
//        auto p = createTestSink();
//        sink->open();
//        sink->write("0000000000000000");
//        sink->seek(0, true);
//                CHECK(sink->tell() == 0);
//        sink->write("1");
//                CHECK(sink->tell() == 1);
//        sink->write("1");
//        delete sink;
//
//        auto source = new FileSource(file);
//        source->open();
//                CHECK(source->readChar() == '1');
//                CHECK(source->tell() == 1);
//                CHECK(source->readChar() == '1');
//        delete source;
//    }
}