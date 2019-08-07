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

using std::shared_ptr;

struct IndexEntryV1;

/**
 * Used to identify a file as a file created by this binary.
 */
extern const uint MAGIC_NUMBER;

extern const u_int64_t kB;

extern const u_int64_t MB;

extern const u_int64_t GB;

extern const u_int64_t TB;

extern const int DEFAULT_RECORD_SIZE;

/**
 * Size of buffer for decompressed data
 * I'd prefer to set the following value as constant ints, but C++ then refuses to initialize arrays with the nice {0}
 * syntax. Though it works, when the constants are in a class.
 */
#define WINDOW_SIZE 32768

/**
 * Chunk size for raw data from compressed file
 */
#define CHUNK_SIZE 16384

/**
 * As we mix strings, stringstreams and cstrings, a buffer with this size is used to ensure, that the decompressed data
 * copied to the cleansed buffer is zero terminated.
 */
#define CLEAN_WINDOW_SIZE (WINDOW_SIZE + 1)

struct VirtualIndexEntry {
    bool operator==(const VirtualIndexEntry &rhs) const { return true; };
};



#endif //FASTQINDEX_COMMONSTRUCTS_H
