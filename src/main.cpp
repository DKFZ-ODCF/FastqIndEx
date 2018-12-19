/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <iostream>

#include "main.h"
#include "Starter.h"

int main(int argc, const char *argv[]) {
    Starter *starter = Starter::getInstance();
    auto runner = starter->createRunner(argc, argv);

    if (!runner->checkPremises()) {
        cerr << "There were errors preventing FastqInDex to start:\n";
        for (auto const &message : runner->getErrorMessages()) {
            cerr << "\t" << message << "\n";
        }
    } else {
        return runner->run();
    }
}