/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "CommonStructsAndConstants.h"

using namespace std;

const u_char MAGIC_NUMBER_RAW[4] = {1, 2, 3, 4};

/**
 * Magic number to identify FastqIndEx index files.
 * Basically (((uint) 4) << 24) + (((uint) 3) << 16) + (((uint) 2) << 8) + 1;
 */
const uint MAGIC_NUMBER = *((uint *) MAGIC_NUMBER_RAW);

const u_int64_t kB = 1024;

const u_int64_t MB = kB * 1024;

const u_int64_t GB = MB * 1024;

const u_int64_t TB = GB * 1024;

const int DEFAULT_RECORD_SIZE = 4;

