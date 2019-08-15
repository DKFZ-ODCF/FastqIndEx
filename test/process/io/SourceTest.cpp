/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/Source.h"
#include "process/io/FileSource.h"
#include "process/io/StreamSource.h"
#include "../../TestResourcesAndFunctions.h"
#include <cstring>
#include <fstream>
#include <UnitTest++/UnitTest++.h>

const char *SUITE_BIS_TESTS = "InputStreamTestSuite";
const char *TEST_STREAM_ISOURCE_OPERATIONS = "Test StreamSource operations";
const char *TEST_STREAM_ISOURCE_SKIP = "Test StreamSource skip on large dataset";
const char *TEST_STREAM_PSOURCE_OPERATIONS = "Test FileSource operations";

path getAndCheckTextFile() {
    path textFile = TestResourcesAndFunctions::getResource("TestTextFile.txt");
            CHECK(exists(textFile));
            CHECK(file_size(textFile) == 67);
    return textFile;
}

void runInputStreamTest(Source *Source, int64_t expectedSize) {
    /**
     * File with the following content:
     * First line
     * Second line
     * Third line
     * Fourth line
     * Fifth line
     * Sixth line
     */
    Byte buf[128]{0};
    Source->open();
    auto readBytes = Source->read(buf, 5);
            CHECK(readBytes == 5);
            CHECK(Source->size() == expectedSize);
            CHECK(string(reinterpret_cast<const char *>( buf)) == string("First"));
    memset(buf, 0, 128);
            CHECK(Source->skip(1) != 0);
            CHECK(Source->readChar() == 'l'); // l of line
            CHECK(Source->skip(4) != 0);
            CHECK(Source->readChar() == 'S'); // S of Second
            CHECK(Source->tell() == 12);
    Source->seek(0, true);
            CHECK(Source->tell() == 0);
            CHECK(Source->readChar() == 'F'); // F of First
            CHECK(Source->close());
}

SUITE (SUITE_BIS_TESTS) {
    TEST (TEST_STREAM_PSOURCE_OPERATIONS) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_PSOURCE_OPERATIONS);
        path textFile = getAndCheckTextFile();

        FileSource Source(textFile);
                CHECK(Source.isFile());


        runInputStreamTest(&Source, file_size(textFile));
    }

    TEST (testCanRead) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
        path testFile = getAndCheckTextFile();
        ifstream testData(testFile);

        StreamSource Source(&testData);

                CHECK(Source.tell() == 0);
                CHECK(Source.canRead());
                CHECK(Source.tell() == 0);
    }

    TEST (testCanReadWontWorkWhenDatasourceIsDepleted) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
        path testFile = getAndCheckTextFile();
        ifstream testData(testFile);

        Bytef buf[32768]{0};
        StreamSource Source(&testData, 4, 4);
        auto read = Source.read(buf, 32768);
                CHECK(!Source.canRead());
                CHECK(read == static_cast<int64_t>(file_size(testFile)));

        // Also, canRead can also be true, if a valid rewind was done.
//                CHECK(false);
    }

    TEST (TEST_STREAM_ISOURCE_OPERATIONS) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
        ifstream testData(getAndCheckTextFile());

        StreamSource Source(&testData);
                CHECK(Source.isStream());

        int64_t expectedSize = -1; // Use overflow to get the max value.

        runInputStreamTest(&Source, expectedSize);
    }

    TEST (TEST_STREAM_ISOURCE_SKIP) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
        ifstream testData(getAndCheckTextFile());

        StreamSource Source(&testData, 4, 4);
        Source.skip(16);
                CHECK(Source.getRewindBufferSize() == 16);
                CHECK(Source.getSegmentsInRewindBuffer() == 4);
        Source.rewind(4);
                CHECK(Source.getSegmentsInRewindBuffer() == 4);
                CHECK(Source.getRewoundBytes() == 4);
        Source.rewind(4);
                CHECK(Source.getRewoundBytes() == 8);
        Source.skip(12);
                CHECK(Source.getRewoundBytes() == 0);
                CHECK(Source.getSegmentsInRewindBuffer() == 4);

    }
}
