/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "StreamInputSource.h"
#include <cstring>

const u_int64_t StreamInputSource::getTotalReadBytes() {
    return InputSource::getTotalReadBytes();
}

bool StreamInputSource::open() {
    return true;
}

bool StreamInputSource::close() {
    return true;
}

bool StreamInputSource::exists() {
    return true;
}

/**
 * What is the proper size for a stream? uint64_t max?
 * @return
 */
uint64_t StreamInputSource::size() {
    return -1;
}

int StreamInputSource::getRewindBufferSize() {
    int total = 0;
    for (auto length : rewindBufferEntryLength) {
        total += length;
    }
    return total;
}

char *StreamInputSource::joinRewindBuffer() {
    char *buf = new char[getRewindBufferSize()];
    int bufPos = 0;
    for (int i = 0; i < rewindBuffer.size(); i++) {
        memcpy(buf + bufPos, rewindBuffer[i], rewindBufferEntryLength[i]);
        bufPos += rewindBufferEntryLength[i];
    }
    return buf;
}

bool StreamInputSource::isFileSource() { return false; }

bool StreamInputSource::isStreamSource() { return true; }

/**
 * Reads numberOfBytes into the targetBuffer.
 * @param targetBuffer targetBuffer is either a valid Byte array OR nullptr! In case of nullptr, the read bytes are
 *                     still stored in the internal rewind buffer. Can e.g. be used by skip.
 * @param numberOfBytes
 * @return
 */
int StreamInputSource::read(Bytef *targetBuffer, int numberOfBytes) {
    if (rewoundBytes > 0) {
        return readFromBuffer(targetBuffer, numberOfBytes);
    } else {
        return readFromStream(targetBuffer, numberOfBytes);
    }
}

int StreamInputSource::readFromBuffer(Bytef *targetBuffer, int numberOfBytes) {
    char *buf = joinRewindBuffer();
    int bufLen = getRewindBufferSize();

    int copiedBytes = numberOfBytes;
    if (numberOfBytes > bufLen) {
        // Cut it, copy less!
        copiedBytes = bufLen;
    }

    if (numberOfBytes > rewoundBytes) {
        copiedBytes = rewoundBytes;
    }

    // Copy copiedBytes of data from the right side!
    if (targetBuffer != nullptr)
        memcpy(targetBuffer, buf + bufLen - rewoundBytes, copiedBytes);
    rewoundBytes -= copiedBytes;
    currentPosition += copiedBytes;

    delete[] buf;

    return copiedBytes;
}

int StreamInputSource::readFromStream(Bytef *targetBuffer, int numberOfBytes) {
    char *readBuffer = new char[numberOfBytes];
    int amountRead = inputStream->readsome(readBuffer, numberOfBytes);
    if (amountRead > 0) {
        if (targetBuffer != nullptr)
            memcpy(targetBuffer, readBuffer, amountRead);
        rewindBuffer.emplace_back(readBuffer);
        rewindBufferEntryLength.emplace_back(amountRead);
        if (rewindBuffer.size() > maxSegmentsInBuffer) {
            char *front = rewindBuffer.front();
            rewindBuffer.pop_front();
            rewindBufferEntryLength.pop_front();
            delete[] front;
        }
        currentPosition += amountRead;          // Advance current position marker
    }
    return amountRead;
}

int StreamInputSource::readChar() {
    Byte result = 0;
    int res = this->read(&result, 1);
    return res < 0 ? res : (int) result;
}

/**
 * A lot of cases apply here:
 * - if absolute:
 *   - nByte > curPos
 *     => Skip to curPos or eof
 *   - nByte < curPos
 *     - nByte < sizeOfBuffer   => Go on
 *     - nByte > sizeOfBuffer   => Error!
 *
 * - else if relative
 *   - nByte > 0
 *     => Skip by nByte
 *   - nByte < 0
 *     - nByte < sizeOfBuffer
 *     - nByte > sizeOfBuffer
 *
 * To make things easier, convert a relative offset to an absolute offset first. So only the cases for absolute apply
 *
 * @param nByte The absolute position in the stream or the relative position to the current position
 * @param absolute Absolute or relative position
 * @return -1 for a failure or the new position in the stream
 */
int StreamInputSource::seek(int64_t nByte, bool absolute) {
    if (!absolute) {
        nByte = currentPosition + nByte;
    }

    if (nByte > currentPosition) {
        return skip(nByte);
    } else {
        uint64_t difference = currentPosition - nByte;
        return rewind(difference);
    }
}

/**
 * Several cases:
 *
 * => Normalize to border
 *
 *
 * @param nByte
 * @return
 */
int StreamInputSource::rewind(uint64_t nByte) {

    // Make sure, rewind won't go over the border.
    if (nByte > currentPosition) {
        nByte = currentPosition;
    }

    // Check, if the rewind is too far
    // We also need to be aware of a previous rewind!
    if (nByte + rewoundBytes > getRewindBufferSize()) {
        return -1; // We cannot go back further than the rewind buffer. This counts as an error
    }

    // All good? Let's do the actual rewind.
    currentPosition -= nByte; // Set position
    rewoundBytes += nByte;

    return nByte;
}

/**
 * Skips nByte bytes and stores them in the buffer for possible rewinds.
 * @param nByte
 * @return
 */
int StreamInputSource::skip(uint64_t nByte) {
    if (nByte == 0) return true;

    int maxDefaultBufferSize = maxSegmentsInBuffer * defaultChunkSizeForReads;
    if (nByte > maxDefaultBufferSize) {
        // Ignore some bytes, build up new buffer.
        inputStream->ignore(nByte - maxDefaultBufferSize);

        for (int i = 0; i < maxSegmentsInBuffer; i++) {
            this->read(nullptr, defaultChunkSizeForReads);
        }
    } else {
        // Fill buffer until nByte is reached.
        uint64_t skip = nByte;
        for (; skip >= defaultChunkSizeForReads; skip -= defaultChunkSizeForReads) {
            this->read(nullptr, defaultChunkSizeForReads);
        }
        if (skip > 0) {
            this->read(nullptr, skip);
        }
    }
    return true;
}

uint64_t StreamInputSource::tell() {
    return currentPosition;
}

int StreamInputSource::lastError() {
    return 0;
}

/**
 * Try to read one more byte from the stream. The streams eof() method won't necessarily work and will return false in
 * most cases I tested.
 * @return
 */
bool StreamInputSource::canRead() {
    Byte buf{0};
    int nByte = read(&buf, 1);
    if(nByte == 0) return false;
    rewind(1);
}
