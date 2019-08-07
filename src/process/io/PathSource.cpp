/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "PathSource.h"

PathSource::PathSource(const path &file) : file(IOHelper::fullPath(file)), lockHandler(file) {
//    fStream = std::ifstream(file, std::ifstream::binary);
}

PathSource::~PathSource() {
    close();
}

bool PathSource::fulfillsPremises() {
    path _path = file;
    bool isValid = exists();
    if (isValid) {
        if (isSymlink())
            _path = read_symlink(_path);

        if (!is_regular_file(_path)) {
            isValid = false;
            addErrorMessage("The path '", toString(), "' does not point to a file.");
        }
    } else {
        addErrorMessage("The file ", toString(), " does not exist.");
    }
    return isValid;
}

bool PathSource::open() {
    if (!fStream.is_open()) {
        fStream.open(file);
        std::ifstream(file, std::ifstream::binary);
    }
    return fStream.is_open();
}

bool PathSource::openWithReadLock() {
    if (!lockHandler.readLock()) {
        return false;
    }
    return open();
}

bool PathSource::close() {
    if (fStream.is_open())
        fStream.close();
    if (lockHandler.hasLock())
        lockHandler.unlock();
    return true;
}

int PathSource::read(Bytef *targetBuffer, int numberOfBytes) {
    fStream.read(reinterpret_cast<char *>(targetBuffer), numberOfBytes);
    int amountRead = fStream.gcount();
    return (int) amountRead;
}

int PathSource::readChar() {
    Byte result = 0;
    int res = this->read(&result, 1);
    return res < 0 ? res : (int) result;
}

int PathSource::seek(int64_t nByte, bool absolute) {
    if (lastError()) {
        // Seek / Read can run over file borders and it might be necessary to just reopen it. We do this here.
        close();
        if (!open()) return 0;
    }

    if (absolute)
        fStream.seekg(nByte, std::ifstream::beg);
    else
        fStream.seekg(nByte, std::ifstream::cur);
    return (!fStream.fail() && !fStream.bad()) ? 1 : 0;
}

int PathSource::skip(uint64_t nBytes) {
    return seek(nBytes, false);
}

uint64_t PathSource::tell() {
    if (fStream.is_open())
        return fStream.tellg();
    return 0;
}

bool PathSource::canRead() {
    return tell() < size();
}

int PathSource::lastError() {
    return fStream.fail() || fStream.bad();
}

bool PathSource::isOpen() {
    return fStream.is_open();
}

bool PathSource::eof() {
    return fStream.eof();
}

bool PathSource::isGood() {
    return fStream.good();
}

bool PathSource::empty() {
    return size() == 0;
}

bool PathSource::canWrite() {
    return false;
}

string PathSource::toString() {
    return file.string();
}

bool PathSource::hasLock() {
    return lockHandler.hasLock();
}

bool PathSource::unlock() {
    lockHandler.unlock();
    return !hasLock();
}

int PathSource::rewind(uint64_t nByte) {
    return seek(-nByte, false);
}
