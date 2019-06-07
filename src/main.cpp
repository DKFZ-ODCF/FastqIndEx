/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>

#include "main.h"
#include "Starter.h"

int main(int argc, const char *argv[]) {
    Starter *starter = Starter::getInstance();
    shared_ptr<Runner> runner;
    runner = starter->createRunner(argc, argv);

    int exitCode = 0;
    if (!runner->checkPremises()) {
        cerr << "There were errors preventing FastqIndEx to start:\n";
        for (auto const &message : runner->getErrorMessages()) {
            cerr << "\t" << message << "\n";
        }
        exitCode = 1;
    } else {
        exitCode = runner->run();
    }
    delete starter;
    return exitCode;
}