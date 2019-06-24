/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_STREAMINPUTSOURCE_H
#define FASTQINDEX_STREAMINPUTSOURCE_H

#include "InputSource.h"
#include <deque>
#include <iostream>

class StreamInputSource : public InputSource {
private:

    istream *inputStream{nullptr};

    uint64_t currentPosition{0};

    /**
     * Last 8 read chunks from the input source.
     */
    deque<char *> rewindBuffer;

    /**
     * Length of each entry in the rewindBuffer.
     */
    deque<int> rewindBufferEntryLength;

    int rewoundBytes{0};

    int maxSegmentsInBuffer{8};

    int defaultChunkSizeForReads{32768};

public:

    explicit StreamInputSource(
            istream *source,
            int maxSegmentsInBuffer = 8,
            int defaultChunkSizeForReads = 32768
    ) : inputStream(source),
        maxSegmentsInBuffer(maxSegmentsInBuffer),
        defaultChunkSizeForReads(defaultChunkSizeForReads) {};

    const u_int64_t getTotalReadBytes() override;

    int skip(uint64_t nByte) override;

    uint64_t tell() override;

    int lastError() override;

    bool open() override;

    bool close() override;

    bool exists() override;

    uint64_t size() override;

    bool isFileSource() override;

    bool isStreamSource() override;

    int read(Bytef *targetBuffer, int numberOfBytes) override;

    int readFromBuffer(Bytef *targetBuffer, int numberOfBytes);

    int readFromStream(Bytef *targetBuffer, int numberOfBytes);

    int readChar() override;

    int seek(int64_t nByte, bool absolute) override;

    int rewind(uint64_t nByte) override;

    int getRewindBufferSize();

    char* joinRewindBuffer();

    int getSegmentsInRewindBuffer() { return rewindBuffer.size(); };

    int getRewoundBytes() { return rewoundBytes; }

    bool canRead() override;
};

#include "InputSource.h"

#endif //FASTQINDEX_STREAMINPUTSOURCE_H
