/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "PathInputSource.h"

PathInputSource::PathInputSource(const path &source) {
    this->source = source;
}

PathInputSource::~PathInputSource() {
    close();
}

bool PathInputSource::open() {
    if (!filePointer)
        this->filePointer = fopen(source.string().c_str(), "rb");
}

bool PathInputSource::close() {
    if (filePointer) {
        fclose(filePointer);
        filePointer = nullptr;
        return true;
    } else
        return false;
}

int PathInputSource::read(Bytef *targetBuffer, int numberOfBytes) {
    size_t read = fread((void *) targetBuffer, 1, numberOfBytes, filePointer);
    if (ferror(filePointer))
        return -1;
    return (int) read;
}

int PathInputSource::readChar() {
    return getc(this->filePointer);
}

int PathInputSource::seek(int64_t nByte, bool absolute) {
    int result = 0;
    if (absolute)
        result = fseeko64(filePointer, nByte, SEEK_SET);
    else
        result = fseeko64(filePointer, nByte, SEEK_CUR);
    return result == 0 ? 1 : 0;
}

int PathInputSource::skip(uint64_t nBytes) {
    return seek(nBytes, false);
}

uint64_t PathInputSource::tell() {
    return ftello64(filePointer);
}

bool PathInputSource::canRead() {
    return tell() < size();
}

int PathInputSource::lastError() {
    return ferror(filePointer);
}
