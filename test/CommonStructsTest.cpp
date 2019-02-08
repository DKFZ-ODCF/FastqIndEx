/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
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
        IndexHeader indexHeader(1, sizeof(IndexEntryV1));
        // Apply sanity checks for the size of the IndexHeader
                CHECK_EQUAL(512, sizeof(IndexHeader));
                CHECK_EQUAL(4, sizeof(indexHeader.indexWriterVersion));
                CHECK_EQUAL(4, sizeof(indexHeader.sizeOfIndexEntry));
                CHECK_EQUAL(4, sizeof(indexHeader.magicNumber));
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
                CHECK_EQUAL(24, sizeof(IndexEntryV1));
                CHECK_EQUAL(4, sizeof(indexEntry.entryNumber));
                CHECK_EQUAL(1, sizeof(indexEntry.bits));
                CHECK_EQUAL(1, sizeof(indexEntry.entryStartsWithLine));
                CHECK_EQUAL(8, sizeof(indexEntry.offset));
                CHECK_EQUAL(8, sizeof(indexEntry.startingLineInEntry));

        // Now initial values
                CHECK_EQUAL(0, indexEntry.entryNumber);
                CHECK_EQUAL(0, indexEntry.offset);
                CHECK_EQUAL(0, indexEntry.bits);
                CHECK_EQUAL(0, indexEntry.startingLineInEntry);
                CHECK_EQUAL(false, indexEntry.entryStartsWithLine);
    }

    TEST (TEST_INDEX_EMPTY_CONSTRUCT1) {
        IndexEntryV1 indexEntry(10, 20, 1, 1000, true);
                CHECK_EQUAL(10, indexEntry.entryNumber);
                CHECK_EQUAL(20, indexEntry.offset);
                CHECK_EQUAL(1, indexEntry.bits);
                CHECK_EQUAL(1000, indexEntry.startingLineInEntry);
                CHECK_EQUAL(true, indexEntry.entryStartsWithLine);
    }

    TEST (TEST_INDEX_COMPARE_HEADERS) {
        IndexHeader h1(1, sizeof(IndexEntryV1));
        IndexHeader h2(1, sizeof(IndexEntryV1));
                CHECK(h1 == h2);

        IndexHeader h3(2, sizeof(IndexEntryV1));
                CHECK(h1 != h3);
                CHECK(h2 != h3);
    }

    TEST (TEST_INDEX_COMPARE_ENTRIES) {
        IndexEntryV1 e1(1, 0, 2, 0, true);
        IndexEntryV1 e2(1, 0, 2, 0, true);

                CHECK(e1 == e2);

        IndexEntryV1 e3(2, 0, 2, 0, true);
                CHECK(e1 != e3);
                CHECK(e2 != e3);
    }
}