/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../src/CommonStructsAndConstants.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char *SUITE_COMMONSTRUCTS_TESTS = "CommonStructsTests";
const char *TEST_INDEX_EMPTY_CONSTRUCT1 = "IndexEntryConstructor1";
const char *TEST_INDEX_EMPTY_CONSTRUCT2 = "IndexEntryConstructor2";
const char *TEST_INDEX_COMPARE_HEADERS = "IndexHeaderComparison";
const char *TEST_INDEX_COMPARE_ENTRIES = "IndexEntryComparison";

SUITE (SUITE_COMMONSTRUCTS_TESTS) {
    TEST (TEST_INDEX_HEADER_CONSTRUCT1) {
        IndexHeader indexHeader(1, sizeof(IndexEntryV1), 2);
        // Apply sanity checks for the size of the IndexHeader
                CHECK_EQUAL(512, sizeof(IndexHeader));
                CHECK_EQUAL(4, sizeof(indexHeader.indexWriterVersion));
                CHECK_EQUAL(4, sizeof(indexHeader.sizeOfIndexEntry));
                CHECK_EQUAL(4, sizeof(indexHeader.magicNumber));
                CHECK_EQUAL(4, sizeof(indexHeader.blockInterval));
                CHECK_EQUAL(62 * 8, sizeof(indexHeader.reserved));

        // Check content
                CHECK_EQUAL(MAGIC_NUMBER, indexHeader.magicNumber);
                CHECK_EQUAL(1, indexHeader.indexWriterVersion);
                CHECK_EQUAL(sizeof(IndexEntryV1), indexHeader.sizeOfIndexEntry);

        bool allZeroed = true;
        int count = sizeof(indexHeader.reserved) / sizeof(ulong);
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
                CHECK_EQUAL(32808, sizeof(IndexEntryV1));
                CHECK_EQUAL(8, sizeof(indexEntry.blockID));
                CHECK_EQUAL(8, sizeof(indexEntry.blockOffsetInRawFile));
                CHECK_EQUAL(8, sizeof(indexEntry.startingLineInEntry));
                CHECK_EQUAL(1, sizeof(indexEntry.bits));
                CHECK_EQUAL(2, sizeof(indexEntry.offsetOfFirstValidLine));
                CHECK_EQUAL(32768, sizeof(indexEntry.dictionary));

        Bytef dictionaryCheck[32768]{0};

        // Now initial values
                CHECK_EQUAL(0, indexEntry.blockID);
                CHECK_EQUAL(0, indexEntry.bits);
                CHECK_EQUAL(0, indexEntry.offsetOfFirstValidLine);
                CHECK_EQUAL(0, indexEntry.blockOffsetInRawFile);
                CHECK_EQUAL(0, indexEntry.startingLineInEntry);

                CHECK_ARRAY_EQUAL(dictionaryCheck, indexEntry.dictionary, sizeof(dictionaryCheck));
    }

    TEST (TEST_INDEX_EMPTY_CONSTRUCT1) {
        IndexEntryV1 indexEntry(1, 10, 5, 20, 1000);
                CHECK_EQUAL(1, indexEntry.bits);
                CHECK_EQUAL(10, indexEntry.blockID);
                CHECK_EQUAL(5, indexEntry.offsetOfFirstValidLine);
                CHECK_EQUAL(20, indexEntry.blockOffsetInRawFile);
                CHECK_EQUAL(1000, indexEntry.startingLineInEntry);
    }

    TEST (TEST_INDEX_COMPARE_HEADERS) {
        IndexHeader h1(1, sizeof(IndexEntryV1), 1);
        IndexHeader h2(1, sizeof(IndexEntryV1), 1);
                CHECK(h1 == h2);

        IndexHeader h3(2, sizeof(IndexEntryV1), 1);
                CHECK(h1 != h3);
                CHECK(h2 != h3);
        IndexHeader h4(1, sizeof(IndexEntryV1), 2);
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