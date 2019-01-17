/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "../src/CommonStructs.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>

const char* SUITE_COMMONSTRUCTS_TESTS = "CommonStructsTests";
const char* TEST_INDEX_EMTRY_CONSTRUCT1 = "IndexEntryConstructor1";
const char* TEST_INDEX_EMTRY_CONSTRUCT2 = "IndexEntryConstructor2";

SUITE (SUITE_COMMONSTRUCTS_TESTS) {
    TEST (TEST_INDEX_HEADER_CONSTRUCT1) {
        IndexHeader indexHeader(1);
                CHECK_EQUAL((((uint)4) << 24 + 3 << ((uint)16) + ((uint)2) << 8 + 1), indexHeader.reserved_3);
                CHECK_EQUAL(1, indexHeader.binary_version);
                CHECK_EQUAL(0, indexHeader.reserved_0);
                CHECK_EQUAL(0, indexHeader.reserved_1);
                CHECK_EQUAL(0, indexHeader.reserved_2);
                CHECK_EQUAL(0, indexHeader.reserved_3);
    }

    TEST (TEST_INDEX_EMTRY_CONSTRUCT2) {
        IndexEntry indexEntry;
                CHECK_EQUAL(0, indexEntry.entry_no);
                CHECK_EQUAL(0, indexEntry.offset);
                CHECK_EQUAL(0, indexEntry.bits);
                CHECK_EQUAL(0, indexEntry.starting_line_in_entry);
                CHECK_EQUAL(0, indexEntry.line_starts_at_pos_0);
    }

    TEST (TEST_INDEX_EMTRY_CONSTRUCT1) {
        IndexEntry indexEntry(10, 20, 1, 1000, 1);
                CHECK_EQUAL(10, indexEntry.entry_no);
                CHECK_EQUAL(20, indexEntry.offset);
                CHECK_EQUAL(1, indexEntry.bits);
                CHECK_EQUAL(1000, indexEntry.starting_line_in_entry);
                CHECK_EQUAL(1, indexEntry.line_starts_at_pos_0);
    }
}