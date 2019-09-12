/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "FileSource.h"

FileSource::FileSource(const path &file) : file(IOHelper::fullPath(file)), lockHandler(file) {}

FileSource::~FileSource() {
    close();
}

bool FileSource::fulfillsPremises() {
    path _path = file;
    bool isValid = exists();
    if (isValid) {
        if (isSymlink())
            _path = read_symlink(_path);

        if (!is_regular_file(_path)) {
            isValid = false;
            addErrorMessage("'", toString(), "' does not point to a file.");
        }
    } else {
        addErrorMessage("File ", toString(), " does not exist.");
    }
    return isValid;
}

bool FileSource::open() {
    if (!fStream.is_open()) {
        fStream.open(file);
        std::ifstream(file, std::ifstream::binary);
    }
    return fStream.is_open();
}

bool FileSource::openWithReadLock() {
    if (!lockHandler.readLock()) {
        return false;
    }
    return open();
}

bool FileSource::close() {
    if (fStream.is_open())
        fStream.close();
    if (lockHandler.hasLock())
        lockHandler.unlock();
    return true;
}

bool FileSource::hasLock() {
    return lockHandler.hasLock();
}

bool FileSource::unlock() {
    lockHandler.unlock();
    return !hasLock();
}

bool FileSource::isOpen() {
    return fStream.is_open();
}

bool FileSource::eof() {
    fStream.peek();
    return fStream.eof() == 1 ? true : false;
}

int64_t FileSource::read(Bytef *targetBuffer, int numberOfBytes) {
    fStream.read(reinterpret_cast<char *>(targetBuffer), numberOfBytes);
    int64_t amountRead = fStream.gcount();
    return amountRead;
}

int FileSource::readChar() {
    Byte result = 0;
    int res = static_cast<int>(this->read(&result, 1));
    return res < 0 ? res : (int) result;
}

int64_t FileSource::seek(int64_t nByte, bool absolute) {
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

int64_t FileSource::skip(int64_t nBytes) {
    return seek(nBytes, false);
}

int64_t FileSource::rewind(int64_t nByte) {
    return seek(-nByte, false);
}

int64_t FileSource::tell() {
    if (fStream.is_open())
        return fStream.tellg();
    return -1;
}

int FileSource::lastError() {
    return fStream.fail() || fStream.bad();
}

string FileSource::toString() {
    return file.string();
}
