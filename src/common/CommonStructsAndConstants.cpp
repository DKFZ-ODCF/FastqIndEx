/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "CommonStructsAndConstants.h"

using namespace std;

char FQI_BINARY[16384]{0};
char S3HELPER_BINARY[16384]{0};

const u_char MAGIC_NUMBER_RAW[4] = {1, 2, 3, 4};

/**
 * Magic number to identify FastqIndEx index files.
 * Basically (((uint) 4) << 24) + (((uint) 3) << 16) + (((uint) 2) << 8) + 1;
 */
const uint MAGIC_NUMBER = *(reinterpret_cast<uint *>(const_cast<u_char *>(MAGIC_NUMBER_RAW)));

const int64_t kB = 1024;

const int64_t MB = kB * 1024;

const int64_t GB = MB * 1024;

const int64_t TB = GB * 1024;

const int DEFAULT_RECORD_SIZE = 4;

uint stoui(std::string value) {
    // Unfortunately, C++ does not offer the stoui method (or stou). Why? Nobody knows. I found
    // the following code snippet here: https://stackoverflow.com/questions/8715213/why-is-there-no-stdstou

    unsigned long intermediate = std::stoul(value);
    if (intermediate > std::numeric_limits<unsigned>::max()) {
        throw std::out_of_range("stou");
    }
    return static_cast<uint>(intermediate);
}

