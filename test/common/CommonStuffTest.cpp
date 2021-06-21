/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/CommonStructsAndConstants.h"
#include <UnitTest++/UnitTest++.h>
#include <string>

const char *const COMMONSTUFF_TESTS = "Test suite for common structures and constants (also methods)";

const char *const TEST_STOUI_VALID = "Test stoui valid";
const char *const TEST_STOUI_INVALID = "Test stoui invalid";

using namespace std;

SUITE (COMMONSTUFF_TESTS) {
    TEST (TEST_STOUI_VALID) {
                CHECK_EQUAL(10U, stoui("10"));
                CHECK_EQUAL(30U, stoui("30"));
    }

    TEST (TEST_STOUI_INVALID) {
        try {
            stoui(to_string((u_int64_t) 0x01ffffffff));
        } catch (const out_of_range &e) {
        }
        try {
            stoui("-10");
        } catch (const out_of_range &e) {
        }
    }
}
