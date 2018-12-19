/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "Runner.h"
#include "Starter.h"
#include <iostream>

using namespace std;

unsigned char ShowStopperRunner::run() {
    cout << *Starter::getInstance()->getCLIOptions();
}


