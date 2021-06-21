/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_SOURCE_H
#define FASTQINDEX_SOURCE_H

#include "common/ErrorAccumulator.h"
#include "process/io/IOBase.h"
#include <experimental/filesystem>
#include <zlib.h>

using namespace std;
using namespace std::experimental::filesystem;

/**
 * The class Source is an extension for either a path or a stream. It allows reading from its source and
 * (to some extent) rewinding. Rewinding is necessary for e.g. the indexer, which needs to jump back in the source, when
 * the stream ended and concatenated files are used.
 */
class Source : public IOBase {
protected:
    int64_t totalReadBytes{0};

    int64_t readStart{0};

    Source() = default;

public:

     ~Source() override = default;

    virtual bool openWithReadLock()  = 0;

    virtual int64_t getTotalReadBytes() { return totalReadBytes; }

    /**
     * Currently only for S3 sources, where we cannot seek in the source stream and need to define something like a
     * range, when opening the file. This method only sets readStart and all code using this will need to be implemented
     * in the target classes.
     */
    virtual void setReadStart(int64_t startBytes) {
        readStart = startBytes;
    }

    /**
     * Read numberOfBytes characters from the underlying input source. Stop, when the source has no more data left.
     * @param targetBuffer The target buffer, where the read data will be placed.
     * @param numberOfBytes The number of characters which shall be read.
     * @return The number of read characters.
     */
    virtual int64_t read(Bytef *targetBuffer, int numberOfBytes) = 0;

    /**
     * Like getc / fgetc, this tries to read one Byte of data from the source.
     * @return -1 or a valid Byte value
     */
    virtual int readChar() = 0;

};

#endif //FASTQINDEX_SOURCE_H
