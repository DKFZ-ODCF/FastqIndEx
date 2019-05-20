/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <cstring>
#include <iostream>
#include <unistd.h>
#include "TestConstants.h"
#include "UnitTest++/UnitTest++.h"
#include "../src/ErrorAccumulator.h"

using namespace std;

const char *SANITY_TEST = "Sanity";

char TEST_BINARY[16384]{0};

/**
 * Switch to detect a proper application flow. Set to true AFTER the tests were run uninterrupted
 */
bool exitIntentionally = false;

// See https://github.com/unittest-cpp/unittest-cpp/wiki/Writing-and-Running-Your-First-Test
TEST (SANITY_TEST) {
            CHECK_EQUAL(1, 1);
}

/**
 * Trap for exit(). Detects, if exitIntentionally is set to true and reports a message if it is not.
 */
void unintentionalTestApplicationExit() {
    if (!exitIntentionally)
        std::cout << "A test failed and forced the testapp to end unintentionally.\n"
                  << "This is probably the case because a method or library called exit, forcing the application to "
                  << "exit, before all tests could run and therefore no test report is available.\n";
}

int main(int argc, const char *argv[]) {
    // Register the exit trap to detect unintentional exits.
    ErrorAccumulator::setVerbosity(3);

    atexit(unintentionalTestApplicationExit);
    strcpy(TEST_BINARY, argv[0]);
    int result = UnitTest::RunAllTests();

    // Tests ran, results were reported, set switch to true to indicate, that the application executed as expected.
    exitIntentionally = true;
    exit(result);
}