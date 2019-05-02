/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/InputSource.h"
#include "../src/PathInputSource.h"
#include "../src/StreamInputSource.h"
#include "TestResourcesAndFunctions.h"
#include <cstring>
#include <fstream>
#include <UnitTest++/UnitTest++.h>

const char *SUITE_BIS_TESTS = "InputStreamTestSuite";
const char *TEST_STREAM_ISOURCE_OPERATIONS = "Test StreamInputSource operations";
const char *TEST_STREAM_ISOURCE_SKIP = "Test StreamInputSource skip on large dataset";
const char *TEST_STREAM_PSOURCE_OPERATIONS = "Test PathInputSource operations";

path getAndCheckTextFile(TestResourcesAndFunctions *res) {
    path textFile = res->getResource("TestTextFile.txt");
            CHECK(exists(textFile));
            CHECK(file_size(textFile) == 67);
    return textFile;
}

void runInputStreamTest(InputSource *inputSource, uint64_t expectedSize) {
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
    inputSource->open();
    inputSource->read(buf, 5);
            CHECK(inputSource->size() == expectedSize);
            CHECK(string((const char *) buf) == string("First"));
    memset(buf, 0, 128);
            CHECK(inputSource->skip(1) != 0);
            CHECK(inputSource->readChar() == 'l'); // l of line
            CHECK(inputSource->skip(4) != 0);
            CHECK(inputSource->readChar() == 'S'); // S of Second
            CHECK(inputSource->tell() == 12);
    inputSource->seek(0, true);
            CHECK(inputSource->tell() == 0);
            CHECK(inputSource->readChar() == 'F'); // F of First
            CHECK(inputSource->close());
}

SUITE (SUITE_BIS_TESTS) {
    TEST (TEST_STREAM_PSOURCE_OPERATIONS) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_PSOURCE_OPERATIONS);
        path textFile = getAndCheckTextFile(&res);

        PathInputSource inputSource(textFile);
                CHECK(inputSource.isFileSource());


        runInputStreamTest(&inputSource, file_size(textFile));
    }

//    TEST (testReadOverBufferBoundaries) {
//        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
//        path testFile = getAndCheckTextFile(&res);
//        ifstream testData(testFile);
//
//        StreamInputSource inputSource(&testData);
//
//        inputSource.skip(file_size(testFile));
//                CHECK(!inputSource.canRead());
//    }

    TEST (testCanReadWontWorkWhenDatasourceIsDepleted) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
        path testFile = getAndCheckTextFile(&res);
        ifstream testData(testFile);

        Bytef buf[32768]{0};
        StreamInputSource inputSource(&testData, 4, 4);
        auto read = inputSource.read(buf, 32768);
                CHECK(!inputSource.canRead());
                CHECK(read == file_size(testFile));

        // Also, canRead can also be true, if a valid rewind was done.
//                CHECK(false);
    }

    TEST (TEST_STREAM_ISOURCE_OPERATIONS) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
        ifstream testData(getAndCheckTextFile(&res));

        StreamInputSource inputSource(&testData);
                CHECK(inputSource.isStreamSource());

        uint64_t expectedSize = -1; // Use overflow to get the max value.

        runInputStreamTest(&inputSource, expectedSize);
    }

    TEST (TEST_STREAM_ISOURCE_SKIP) {
        TestResourcesAndFunctions res(SUITE_BIS_TESTS, TEST_STREAM_ISOURCE_OPERATIONS);
        ifstream testData(getAndCheckTextFile(&res));

        StreamInputSource inputSource(&testData, 4, 4);
        inputSource.skip(16);
                CHECK(inputSource.getRewindBufferSize() == 16);
                CHECK(inputSource.getSegmentsInRewindBuffer() == 4);
        inputSource.rewind(4);
                CHECK(inputSource.getSegmentsInRewindBuffer() == 4);
                CHECK(inputSource.getRewoundBytes() == 4);
        inputSource.rewind(4);
                CHECK(inputSource.getRewoundBytes() == 8);
        inputSource.skip(12);
                CHECK(inputSource.getRewoundBytes() == 0);
                CHECK(inputSource.getSegmentsInRewindBuffer() == 4);

    }
}
