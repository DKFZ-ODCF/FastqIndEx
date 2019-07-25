/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

const char *const PATH_SINK_TEST_SUITE = "Test suite for the PathSink class";
const char *const PATH_SINK_CONSTRUCT = "Test construction and checkPremises()";
const char *const PATH_SINK_OPEN = "Test open and close";
const char *const PATH_SINK_OPENLOCKED = "Test open and close with lock_unlock";
const char *const PATH_SINK_AQUIRELOCK_LATER = "Test aquire lock after file open";
const char *const PATH_SINK_WRITE_TELL_SEEK = "Test write tell seek rewind functions";
const char *const PATH_SINK_WRITE_OVERWRITEBYTES = "Test write rewind_seek overwrite bytes";

#include "process/io/PathSink.h"
#include "process/io/PathSource.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

SUITE (PATH_SINK_TEST_SUITE) {
    TEST (PATH_SINK_CONSTRUCT) {
        TestResourcesAndFunctions res(PATH_SINK_TEST_SUITE, PATH_SINK_CONSTRUCT);

        auto ps = res.filePath(TEST_FASTQ_SMALL);
        PathSink p(ps);
                CHECK(p.checkPremises());
                CHECK(p.getPath() == ps);
                CHECK(!p.exists());
                CHECK(!p.hasLock());
                CHECK(!p.isOpen());
                CHECK(p.isGood());
                CHECK(!p.isBad());
                CHECK(!p.eof());
                CHECK(p.isFile());
                CHECK(!p.isStream());
                CHECK(p.toString() == ps.string());
                CHECK(p.size() == 0);
                CHECK(p.tell() == 0);
                CHECK(p.getErrorMessages().empty());
                CHECK(!p.canRead());
                CHECK(p.canWrite());

        auto p2 = PathSink(res.createEmptyFile("noOverwrite"));
                CHECK(p2.exists());
                CHECK(!p2.checkPremises());

        auto p3 = PathSink(res.createEmptyFile("overwrite"), true);
                CHECK(p3.exists());
                CHECK(p3.checkPremises());
    }


    TEST (PATH_SINK_OPEN) {
        TestResourcesAndFunctions res(PATH_SINK_TEST_SUITE, PATH_SINK_OPEN);

        auto ps = res.createEmptyFile(TEST_FASTQ_SMALL);
        PathSink p(ps);

        p.open();
                CHECK(p.isOpen());
        p.close();
                CHECK(!p.isOpen());
    }

    TEST (PATH_SINK_OPENLOCKED) {
        TestResourcesAndFunctions res(PATH_SINK_TEST_SUITE, PATH_SINK_OPENLOCKED);
        auto ps = res.createEmptyFile(TEST_FASTQ_SMALL);
        PathSink p(ps);
                CHECK(!p.isOpen());

        p.openWithWriteLock();

                CHECK(p.isOpen());
                CHECK(p.hasLock());

        p.close();
                CHECK(!p.isOpen());
                CHECK(!p.hasLock());
    }

    TEST (PATH_SINK_AQUIRELOCK_LATER) {
        TestResourcesAndFunctions res(PATH_SINK_TEST_SUITE, PATH_SINK_AQUIRELOCK_LATER);

        auto ps = res.createEmptyFile(TEST_FASTQ_SMALL);
        PathSink p(ps);
                CHECK(!p.isOpen());

        p.open();
                CHECK(!p.hasLock());
                CHECK(p.isOpen());

        p.openWithWriteLock();
                CHECK(p.hasLock());
                CHECK(p.isOpen());
    }

    TEST (PATH_SINK_WRITE_TELL_SEEK) {
        TestResourcesAndFunctions res(PATH_SINK_TEST_SUITE, PATH_SINK_WRITE_TELL_SEEK);

        auto ps = res.filePath("testWrite");
        PathSink p(ps);
        p.openWithWriteLock();

        string test1("AShortMessage");
        p.write(test1);

                CHECK(p.tell() == test1.length());
        p.seek(0, true);
                CHECK(p.tell() == 0);
        p.seek(test1.length(), false);
                CHECK(p.tell() == test1.length());
        p.rewind(2);
                CHECK(p.tell() == test1.length() - 2);

        p.seek(p.size() + 1, true);
                CHECK(p.tell() == test1.length() + 1); // This is only working, because of our opening mode!
//                CHECK(p.eof());                            // This is not working, because of our opening mode!

        const char *c_str = test1.c_str();
        p.write(c_str, 4);
                CHECK(p.tell() == test1.length() + 5);

        p.write(c_str);
                CHECK(p.tell() == 2 * test1.length() + 5);
    }

    TEST (PATH_SINK_WRITE_OVERWRITEBYTES) {
        TestResourcesAndFunctions res(PATH_SINK_TEST_SUITE, PATH_SINK_WRITE_OVERWRITEBYTES);

        auto file = res.createEmptyFile("abc.txt");
        auto sink = new PathSink(file);
        sink->open();
        sink->write("0000000000000000");
        sink->seek(0, true);
                CHECK(sink->tell() == 0);
        sink->write("1");
                CHECK(sink->tell() == 1);
        sink->write("1");
        delete sink;

        auto source = new PathSource(file);
        source->open();
                CHECK(source->readChar() == '1');
                CHECK(source->tell() == 1);
                CHECK(source->readChar() == '1');
        delete source;
    }
}