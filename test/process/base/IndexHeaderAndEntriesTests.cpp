/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/CommonStructsAndConstants.h"
#include "process/base/IndexHeader.h"
#include <UnitTest++/UnitTest++.h>

const char *SUITE_COMMONSTRUCTS_TESTS = "CommonStructsTests";
const char *TEST_INDEX_EMPTY_CONSTRUCT1 = "IndexEntryConstructor1";
const char *TEST_INDEX_EMPTY_CONSTRUCT2 = "IndexEntryConstructor2";
const char *TEST_INDEX_COMPARE_HEADERS = "IndexHeaderComparison";
const char *TEST_INDEX_COMPARE_ENTRIES = "IndexEntryComparison";

SUITE (SUITE_COMMONSTRUCTS_TESTS) {
    TEST (TEST_INDEX_HEADER_CONSTRUCT1) {
        IndexHeader indexHeader(1, sizeof(IndexEntryV1), 2, true);
        // Apply sanity checks for the size of the IndexHeader
                CHECK_EQUAL(512U, sizeof(IndexHeader));
                CHECK_EQUAL(4U, sizeof(indexHeader.indexWriterVersion));
                CHECK_EQUAL(4U, sizeof(indexHeader.sizeOfIndexEntry));
                CHECK_EQUAL(4U, sizeof(indexHeader.magicNumber));
                CHECK_EQUAL(4U, sizeof(indexHeader.blockInterval));
                CHECK_EQUAL(8U, sizeof(indexHeader.numberOfEntries));
                CHECK_EQUAL(8U, sizeof(indexHeader.linesInIndexedFile));
                CHECK_EQUAL(1U, sizeof(indexHeader.dictionariesAreCompressed));
                CHECK_EQUAL(59U * 8, sizeof(indexHeader.reserved));

        // Check content
                CHECK_EQUAL(MAGIC_NUMBER, indexHeader.magicNumber);
                CHECK_EQUAL(1U, indexHeader.indexWriterVersion);
                CHECK_EQUAL(sizeof(IndexEntryV1), indexHeader.sizeOfIndexEntry);

        bool allZeroed = true;
        int count = sizeof(indexHeader.reserved) / sizeof(int64_t);
        for (int i = 0; i < count; i++) {
            allZeroed = allZeroed & (indexHeader.reserved[i] == 0);
        }
                CHECK(allZeroed);
    }

    TEST (TEST_INDEX_EMPTY_CONSTRUCT2) {
        IndexEntryV1 indexEntry;
        // Perform some sanity checks first to make sure, that the format did not change
        // by accident. Be aware, that the struct V1 will have a padding of 2Byte applied.
        // Its size is 2Byte higher than the accumulated size of the single elements.
                CHECK_EQUAL(32800U, sizeof(IndexEntryV1));
                CHECK_EQUAL(8U, sizeof(indexEntry.blockIndex));
                CHECK_EQUAL(8U, sizeof(indexEntry.blockOffsetInRawFile));
                CHECK_EQUAL(8U, sizeof(indexEntry.startingLineInEntry));
                CHECK_EQUAL(1U, sizeof(indexEntry.bits));
                CHECK_EQUAL(4U, sizeof(indexEntry.offsetToNextLineStart));
                CHECK_EQUAL(WINDOW_SIZE, sizeof(indexEntry.dictionary));

        Bytef dictionaryCheck[WINDOW_SIZE]{0};

        // Now initial values
                CHECK_EQUAL(0U, indexEntry.blockIndex);
                CHECK_EQUAL(0U, indexEntry.bits);
                CHECK_EQUAL(0U, indexEntry.offsetToNextLineStart);
                CHECK_EQUAL(0U, indexEntry.blockOffsetInRawFile);
                CHECK_EQUAL(0U, indexEntry.startingLineInEntry);

                CHECK_ARRAY_EQUAL(dictionaryCheck, indexEntry.dictionary, sizeof(dictionaryCheck));
    }

    TEST (TEST_INDEX_EMPTY_CONSTRUCT1) {
        IndexEntryV1 indexEntry(1, 10, 5, 20, 1000);
                CHECK_EQUAL(1U, indexEntry.bits);
                CHECK_EQUAL(10U, indexEntry.blockIndex);
                CHECK_EQUAL(5U, indexEntry.offsetToNextLineStart);
                CHECK_EQUAL(20U, indexEntry.blockOffsetInRawFile);
                CHECK_EQUAL(1000U, indexEntry.startingLineInEntry);
    }

    TEST (TEST_INDEX_COMPARE_HEADERS) {
        IndexHeader h1(1, sizeof(IndexEntryV1), 1, true);
        IndexHeader h2(1, sizeof(IndexEntryV1), 1, true);
                CHECK(h1 == h2);

        IndexHeader h3(2, sizeof(IndexEntryV1), 1, true);
                CHECK(h1 != h3);
                CHECK(h2 != h3);
        IndexHeader h4(1, sizeof(IndexEntryV1), 2, true);
                CHECK(h1 != h4);
                CHECK(h2 != h4);
    }

    TEST (TEST_INDEX_COMPARE_ENTRIES) {
        IndexEntryV1 e1(0, 1, 2, 0, 0);
        IndexEntryV1 e2(0, 1, 2, 0, 0);

                CHECK(e1 == e2);

        IndexEntryV1 e3(0, 1, 2, 0, 1);
                CHECK(e1 != e3);
                CHECK(e2 != e3);

        IndexEntryV1 e4(0, 2, 2, 0, 0);
                CHECK(e1 != e4);
                CHECK(e2 != e4);
    }
}