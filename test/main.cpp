/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <cstring>
#include "TestConstants.h"
#include "UnitTest++/UnitTest++.h"

using namespace std;

const char *SANITY_TEST = "Sanity";

char TEST_BINARY[16384]{0};

// See https://github.com/unittest-cpp/unittest-cpp/wiki/Writing-and-Running-Your-First-Test
TEST (SANITY_TEST) {
            CHECK_EQUAL(1, 1);
}

int main(int argc, const char *argv[]) {
    strcpy(TEST_BINARY, argv[0]);
    return UnitTest::RunAllTests();
}