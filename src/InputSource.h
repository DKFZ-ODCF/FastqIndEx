/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INPUTSOURCE_H
#define FASTQINDEX_INPUTSOURCE_H

#include <experimental/filesystem>
#include <zlib.h>

using namespace std;
using namespace std::experimental::filesystem;

/**
 * The class InputSource is an extension for either a path or a stream. It allows reading from its source and
 * (to some extent) rewinding. Rewinding is necessary for e.g. the indexer, which needs to jump back in the source, when
 * the stream ended and concatenated files are used.
 */
class InputSource {
protected:
    uint64_t totalReadBytes{0};

    InputSource();

    virtual ~InputSource();

public:

    virtual bool open() {};

    virtual bool close() {};

    virtual bool exists() { return false; };

    virtual bool isFileSource() = 0;

    virtual bool isStreamSource() = 0;

    virtual uint64_t size() { return 0; };

    virtual const u_int64_t getTotalReadBytes() { return totalReadBytes; }

    /**
     * Read numberOfBytes characters from the underlying input source. Stop, when the source has no more data left.
     * @param targetBuffer The target buffer, where the read data will be placed.
     * @param numberOfBytes The number of characters which shall be read.
     * @return The number of read characters.
     */
    virtual int read(Bytef *targetBuffer, int numberOfBytes) = 0;

    /**
     * Like getc / fgetc, this tries to read one Byte of data from the source.
     * @return -1 or a valid Byte value
     */
    virtual int readChar() = 0;

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

    /**
     * Get the current position in the source.
     * @return
     */
    virtual uint64_t tell() = 0;

    virtual bool canRead() = 0;

    virtual int lastError() = 0;
};

#endif //FASTQINDEX_INPUTSOURCE_H
