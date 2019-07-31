/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "StreamSource.h"
#include <cstring>


const u_int64_t StreamSource::getTotalReadBytes() {
    return Source::getTotalReadBytes();
}

bool StreamSource::open() {
    return true;
}

bool StreamSource::close() {
    return true;
}

bool StreamSource::exists() {
    return true;
}

/**
 * What is the proper size for a stream? uint64_t max?
 * @return
 */
uint64_t StreamSource::size() {
    return -1;
}

int StreamSource::getRewindBufferSize() {
    int total = 0;
    for (auto length : rewindBufferEntryLength) {
        total += length;
    }
    return total;
}

char *StreamSource::joinRewindBuffer() {
    char *buf = new char[getRewindBufferSize()];
    int bufPos = 0;
    for (int i = 0; i < rewindBuffer.size(); i++) {
        memcpy(buf + bufPos, rewindBuffer[i], rewindBufferEntryLength[i]);
        bufPos += rewindBufferEntryLength[i];
    }
    return buf;
}

bool StreamSource::isFile() { return false; }

bool StreamSource::isStream() { return true; }

/**
 * Reads numberOfBytes into the targetBuffer.
 * @param targetBuffer targetBuffer is either a valid Byte array OR nullptr! In case of nullptr, the read bytes are
 *                     still stored in the internal rewind buffer. Can e.g. be used by skip.
 * @param numberOfBytes
 * @return
 */
int StreamSource::read(Bytef *targetBuffer, int numberOfBytes) {
    if (rewoundBytes > 0) {
        return readFromBuffer(targetBuffer, numberOfBytes);
    } else {
        return readFromStream(targetBuffer, numberOfBytes);
    }
}

int StreamSource::readFromBuffer(Bytef *targetBuffer, int numberOfBytes) {
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

int StreamSource::readFromStream(Bytef *targetBuffer, int numberOfBytes) {
    char *readBuffer = new char[numberOfBytes]{0};
    inputStream->sync();

    inputStream->read(readBuffer, numberOfBytes);
    int amountRead = inputStream->gcount();

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
    } else {
        delete[] readBuffer;
    }
    return amountRead;
}

int StreamSource::readChar() {
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
int StreamSource::seek(int64_t nByte, bool absolute) {
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
 * @param nByte
 * @return
 */
int StreamSource::rewind(uint64_t nByte) {

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
int StreamSource::skip(uint64_t nByte) {
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

uint64_t StreamSource::tell() {
    return currentPosition;
}

int StreamSource::lastError() {
    return 0;
}


/**
 * Peek and check for eof()
 * @return
 */
bool StreamSource::canRead() {
    this->inputStream->peek();
    return !this->inputStream->eof();
}

bool StreamSource::checkPremises() {
    return false;
}

bool StreamSource::isOpen() {
    return false;
}

bool StreamSource::eof() {
    return false;
}

bool StreamSource::isGood() {
    return false;
}

bool StreamSource::isSymlink() {
    return false;
}

bool StreamSource::empty() {
    return false;
}

bool StreamSource::canWrite() {
    return false;
}

string StreamSource::toString() {
    return std::__cxx11::string();
}

bool StreamSource::openWithReadLock() {
    return false;
}
