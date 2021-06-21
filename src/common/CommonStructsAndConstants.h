/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_COMMONSTRUCTS_H
#define FASTQINDEX_COMMONSTRUCTS_H

#include <cstring>
#include <memory>
#include <string>
#include <zconf.h>

/**
 * In any case, this value would need to be const. But as we do not have the value when the variable is created, we
 * will just keep it here. At the end, it only stores the path to the test binary and is used for tests only.
 */
extern char FQI_BINARY[16384];
extern char S3HELPER_BINARY[16384];

/**
 * Used to identify a file as a file created by this binary.
 */
extern const uint MAGIC_NUMBER;

extern const int64_t kB;

extern const int64_t MB;

extern const int64_t GB;

extern const int64_t TB;

extern const int DEFAULT_RECORD_SIZE;

/**
 * Size of buffer for decompressed data
 * I'd prefer to set the following value as constant ints, but C++ then refuses to initialize arrays with the nice {0}
 * syntax. Though it works, when the constants are in a class.
 */
#define WINDOW_SIZE 32768U

/**
 * Chunk size for raw data from compressed file
 */
#define CHUNK_SIZE 16384U

/**
 * As we mix strings, stringstreams and cstrings, a buffer with this size is used to ensure, that the decompressed data
 * copied to the cleansed buffer is zero terminated.
 */
#define CLEAN_WINDOW_SIZE (WINDOW_SIZE + 1)

/**
 * stoui is not available as a core method (unlike stoi or stol ...). Mimic it here.
 *
 * Throws an out_of_range excpetion, if the value is too large
 * @param value A string
 * @return The hopefully converted value.
 */
uint stoui(std::string value);


#endif //FASTQINDEX_COMMONSTRUCTS_H
