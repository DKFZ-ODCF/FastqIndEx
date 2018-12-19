/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "UnitTest++/UnitTest++.h"

// See https://github.com/unittest-cpp/unittest-cpp/wiki/Writing-and-Running-Your-First-Test
TEST(Sanity)
{
            CHECK_EQUAL(1, 1);
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}