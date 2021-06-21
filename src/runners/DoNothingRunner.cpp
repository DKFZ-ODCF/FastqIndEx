/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "DoNothingRunner.h"

using namespace std;

unsigned char DoNothingRunner::_run() {
//    cerr << *Starter::getInstance()->getCLIOptions();
    return 0;
}
