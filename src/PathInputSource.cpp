/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "PathInputSource.h"

PathInputSource::PathInputSource(const path &source) {
    this->source = source;
    fPointer = std::ifstream(source, std::ifstream::binary);
}

PathInputSource::~PathInputSource() {
    close();
}

bool PathInputSource::open() {
    debug("Opening path input source from file " + this->source.string());
    if (!fPointer.is_open())
        fPointer.open(source);
    return fPointer.is_open();
}

bool PathInputSource::close() {
    debug("Closing path in put source.");
    if (fPointer.is_open())
        fPointer.close();
    return true;
}

int PathInputSource::read(Bytef *targetBuffer, int numberOfBytes) {
    fPointer.read(reinterpret_cast<char *>(targetBuffer), numberOfBytes);
    int amountRead = fPointer.gcount();
    return (int) amountRead;
}

int PathInputSource::readChar() {
    Byte result = 0;
    int res = this->read(&result, 1);
    return res < 0 ? res : (int) result;
}

int PathInputSource::seek(int64_t nByte, bool absolute) {
    if(lastError()) {
        // Seek / Read can run over file borders and it might be necessary to just reopen it. We do this here.
        close();
        if(!open()) return 0;
    }

    if (absolute)
        fPointer.seekg(nByte, std::ifstream::beg);
    else
        fPointer.seekg(nByte, std::ifstream::cur);
    return (!fPointer.fail() && !fPointer.bad()) ? 1 : 0;
}

int PathInputSource::skip(uint64_t nBytes) {
    return seek(nBytes, false);
}

uint64_t PathInputSource::tell() {
    return fPointer.tellg();
}

bool PathInputSource::canRead() {
    return tell() < size();
}

int PathInputSource::lastError() {
    return fPointer.fail() || fPointer.bad();
}
