/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_STREAMSOURCE_H
#define FASTQINDEX_STREAMSOURCE_H

#include "Source.h"
#include <deque>
#include <iostream>

class StreamSource : public Source {

protected:

    StreamSource() = default;

    istream *inputStream{nullptr};

    int64_t currentPosition{0};

    /**
     * Last 8 read chunks from the input source.
     */
    deque<char *> rewindBuffer;

    /**
     * Length of each entry in the rewindBuffer.
     */
    deque<int> rewindBufferEntryLength;

    int64_t rewoundBytes{0};

    int maxSegmentsInBuffer{8};

    int defaultChunkSizeForReads{32768};

public:

    static shared_ptr<StreamSource> from(
            istream *source,
            int maxSegmentsInBuffer = 8,
            int defaultChunkSizeForReads = 32768) {
        return make_shared<StreamSource>(source, maxSegmentsInBuffer, defaultChunkSizeForReads);
    }

    /**
     * ifstream behaves different when read() is called than istream
     * for ifstream, read() will return the number of read Bytes, for istream you need gcount()
     */
    explicit StreamSource(
            istream *source,
            int maxSegmentsInBuffer = 8,
            int defaultChunkSizeForReads = 32768
    ) : inputStream(source),
        maxSegmentsInBuffer(maxSegmentsInBuffer),
        defaultChunkSizeForReads(defaultChunkSizeForReads) {};

    int64_t getTotalReadBytes() override;

    /**
     * Returns the pointer to the used stream.
     * @return
     */
    istream *getStream() {        return inputStream;    }

    int64_t getRewindBufferSize();

    int64_t getSegmentsInRewindBuffer() { return rewindBuffer.size(); };

    int64_t getRewoundBytes() { return rewoundBytes; }

    bool fulfillsPremises() override;

    bool open() override;

    bool openWithReadLock() override;

    bool close() override;

    bool isOpen() override;

    bool eof() override;

    bool isGood() override;

    bool isFile() override;

    bool isStream() override;

    bool isSymlink() override;

    bool exists() override;

    int64_t size() override;

    bool empty() override;

    bool canRead() override;

    bool canWrite() override;

    int64_t read(Bytef *targetBuffer, int numberOfBytes) override;

    int64_t readFromBuffer(Bytef *targetBuffer, int numberOfBytes);

    int64_t readFromStream(Bytef *targetBuffer, int numberOfBytes);

    int readChar() override;

    char *joinRewindBuffer();

    int64_t seek(int64_t nByte, bool absolute) override;

    int64_t skip(int64_t nBytes) override;

    int64_t rewind(int64_t nByte) override;

    int64_t tell() override;

    int lastError() override;

    string toString() override;

};

#include "Source.h"

#endif //FASTQINDEX_STREAMSOURCE_H
