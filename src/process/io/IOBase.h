/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_IOBASE_H
#define FASTQINDEX_IOBASE_H

#include "common/ErrorAccumulator.h"
#include <string>

using namespace std;

class IOBase : public ErrorAccumulator {

public:

    virtual bool fulfillsPremises() = 0;

    virtual bool open() = 0;

    virtual bool close() = 0;

    virtual bool hasLock() { return true; };

    virtual bool unlock() { return true; };

    virtual bool isOpen() = 0;

    virtual bool eof() = 0;

    virtual bool isGood() = 0;

    bool isBad() { return !isGood(); }

    virtual bool isFile() = 0;

    virtual bool isStream() = 0;

    virtual bool isSymlink() = 0;

    virtual bool exists() = 0;

    virtual u_int64_t size() = 0;

    virtual bool empty() = 0;

    virtual bool canRead() = 0;

    virtual bool canWrite() = 0;

    /**
     * Upon the next read, use n more bytes from the last read(s) up to the maximum fill state of the rewindBuffer.
     * @param bytes
     * @return Number of rewound bytes
     */
    virtual int seek(int64_t nByte, bool absolute) = 0;

    /**
     * Seek forward the amount of nByte from the current position.
     * @param nByte Number of Bytes to skip forward.
     * @return The number of Bytes which were skipped.
     */
    virtual int skip(uint64_t nByte) = 0;

    virtual int rewind(uint64_t nByte) {
        return seek(-nByte, false);
    }

    virtual string toString() = 0;

    /**
     * Get the current position in the source.
     * @return
     */
    virtual uint64_t tell() = 0;

    virtual int lastError() = 0;
};

#endif //FASTQINDEX_IOBASE_H
