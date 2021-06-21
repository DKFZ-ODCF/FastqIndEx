/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "FileSink.h"

#include <experimental/filesystem>

using namespace std;
using namespace std::experimental::filesystem;

FileSink::FileSink(const path &file, bool forceOverwrite) :
        Sink(forceOverwrite),
        file(IOHelper::fullPath(file)),
        lockHandler(file) {
}

bool FileSink::fulfillsPremises() {
    if (!forceOverwrite && exists()) {
        addErrorMessage(
                "The file '", file, "' exists and cannot be overwritten. Allow overwriting with -w.");
        return false;
    }
    if(!canWrite()) {
        return false;
    }
    return true;
}

bool FileSink::open() {
    if (!isOpen())
        this->fStream.open(file, ios_base::out | ios_base::in | ios_base::binary);
    return isOpen();
}

bool FileSink::openWithWriteLock() {
    bool gotLock = lockHandler.writeLock();
    if (!gotLock)
        return false;
    if (!open()) {
        lockHandler.unlock();
        return false;
    }
    return true;
}

void FileSink::write(const char *message) {
    write(string(message));
}

void FileSink::write(const char *message, int streamSize) {
    if (!isOpen()) {
        addErrorMessage("BUG: You cannot write to a closed file.");
        return;
    }
    this->fStream.write(message, streamSize);
}

void FileSink::write(const string &message) {
    if (!isOpen()) {
        addErrorMessage("BUG: You cannot write to a closed file.");
        return;
    }
    this->fStream.write(message.c_str(), message.length());
}

void FileSink::flush() {
    if (fStream && fStream.is_open()) {
        fStream.flush();
    }
}

bool FileSink::close() {
    if (fStream.is_open())
        fStream.close();
    if (lockHandler.hasLock())
        lockHandler.unlock();
    return true;

}

bool FileSink::isOpen() {
    return fStream.is_open();
}

bool FileSink::eof() {
    return fStream.eof();
}

bool FileSink::isGood() {
    return fStream.good();
}

bool FileSink::isFile() {
    return true;
}

bool FileSink::isStream() {
    return false;
}

bool FileSink::isSymlink() {
    return false;
}

bool FileSink::exists() {
    return std::experimental::filesystem::exists(file);
}

int64_t FileSink::size() {
    if (exists())
        return file_size(file);
    return 0;
}

bool FileSink::empty() {
    return size() == 0;
}

bool FileSink::canRead() {
    return tell() < size();
}

bool FileSink::canWrite() {
    return IOHelper::checkFileWriteability(this->file, "output", this);
}

int64_t FileSink::seek(int64_t nByte, bool absolute) {
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

int64_t FileSink::skip(int64_t nBytes) {
    return seek(nBytes, false);
}

string FileSink::toString() {
    return file.string();
}

int64_t FileSink::tell() {
    if (fStream.is_open())
        return fStream.tellg();
    return 0;
}

int FileSink::lastError() {
    return fStream.fail() || fStream.bad();
}

vector<string> FileSink::getErrorMessages() {
    return concatenateVectors(ErrorAccumulator::getErrorMessages(), lockHandler.getErrorMessages());
}

bool FileSink::hasLock() {
    return lockHandler.hasLock();
}

bool FileSink::unlock() {
    lockHandler.unlock();
    return !hasLock();
}

int64_t FileSink::rewind(int64_t nByte) {
    return seek(-nByte, false);
}
