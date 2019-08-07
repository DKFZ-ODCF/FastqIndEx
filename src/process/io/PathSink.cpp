/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "PathSink.h"

#include <experimental/filesystem>

using namespace std;
using namespace std::experimental::filesystem;

PathSink::PathSink(const path &file, bool forceOverwrite) :
        Sink(forceOverwrite),
        file(IOHelper::fullPath(file)),
        lockHandler(file) {
}

bool PathSink::fulfillsPremises() {
    if (!forceOverwrite && exists()) {
        addErrorMessage(
                "The file '", file, "' exists and cannot be overwritten. Allow overwriting with -w.");
        return false;
    }
    if (exists() && !canWrite()) {
        addErrorMessage("The result file '", file, "' exists and cannot be overwritten. Check its file access rights.");
        return false;
    }
//    if (!exists() && access(parentPath().string().c_str(), W_OK) != 0) {
//        addErrorMessage("The result file cannot be written. Check the access rights of the parent folder.");
//        return false;
//    }
    return true;
}

bool PathSink::open() {
    if (!isOpen())
        this->fStream.open(file, ios_base::out | ios_base::in | ios_base::binary);
    return isOpen();
}

bool PathSink::openWithWriteLock() {
    bool gotLock = lockHandler.writeLock();
    if (!gotLock)
        return false;
    if (!open()) {
        lockHandler.unlock();
        return false;
    }
    return true;
}

void PathSink::write(const char *message) {
    write(string(message));
}

void PathSink::write(const char *message, int streamSize) {
    if (!isOpen()) {
        addErrorMessage("You cannot write to a closed file.");
        return;
    }
    this->fStream.write(message, streamSize);
}

void PathSink::write(const string &message) {
    if (!isOpen()) {
        addErrorMessage("You cannot write to a closed file.");
        return;
    }
    this->fStream.write(message.c_str(), message.length());
}

void PathSink::flush() {
    if (fStream && fStream.is_open()) {
        fStream.flush();
    }
}

bool PathSink::close() {
    if (fStream.is_open())
        fStream.close();
    if (lockHandler.hasLock())
        lockHandler.unlock();
    return true;

}

bool PathSink::isOpen() {
    return fStream.is_open();
}

bool PathSink::eof() {
    return fStream.eof();
}

bool PathSink::isGood() {
    return fStream.good();
}

bool PathSink::isFile() {
    return true;
}

bool PathSink::isStream() {
    return false;
}

bool PathSink::isSymlink() {
    return false;
}

bool PathSink::exists() {
    return std::experimental::filesystem::exists(file);
}

u_int64_t PathSink::size() {
    if (exists())
        return file_size(file);
    return 0;
}

bool PathSink::empty() {
    return size() == 0;
}

bool PathSink::canRead() {
    return tell() < size();
}

bool PathSink::canWrite() {
    return access(toString().c_str(), W_OK) == 0;
}

int PathSink::seek(int64_t nByte, bool absolute) {
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

int PathSink::skip(uint64_t nBytes) {
    return seek(nBytes, false);
}

string PathSink::toString() {
    return file.string();
}

uint64_t PathSink::tell() {
    if (fStream.is_open())
        return fStream.tellg();
    return 0;
}

int PathSink::lastError() {
    return fStream.fail() || fStream.bad();
}

vector<string> PathSink::getErrorMessages() {
    return mergeToNewVector(ErrorAccumulator::getErrorMessages(), lockHandler.getErrorMessages());
}

bool PathSink::hasLock() {
    return lockHandler.hasLock();
}

bool PathSink::unlock() {
    lockHandler.unlock();
    return !hasLock();
}

int PathSink::rewind(uint64_t nByte) {
    return seek(-nByte, false);
}
