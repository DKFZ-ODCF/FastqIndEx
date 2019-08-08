/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */
#include "PathLockHandler.h"
#include <mutex>
#include <sys/file.h>
#include <utility>
using std::lock_guard;
using std::mutex;

PathLockHandler::PathLockHandler(path file)  {
    this->lockedFile = std::move(file);
}

path PathLockHandler::getIndexFile() {
    return lockedFile;
}

PathLockHandler::~PathLockHandler() {
    unlock();
}

/**
 * The methods looks a bit complicated at first glance, but we need some second form of locking
 * when it comes to intra-process locks. It happened in the tests, that the sharable mutex was closed but
 * the write lock could not be established. Using the slightly more complicated syntax below, it works in
 * the tests.
 */
bool PathLockHandler::readLock() {
    lock_guard<mutex> lock(methodMutex);
    if (this->readLockActive)
        return true;
    lockedFileHandle = fopen(lockedFile.c_str(), "rb");
    // clang-tidy will complain about the bitwise operation in the next step. Ignore this.
    bool result = flock(fileno(lockedFileHandle), LOCK_SH | LOCK_NB) == 0;
    if (result)
        this->readLockActive = true;
    else {
        fclose(lockedFileHandle);
        lockedFileHandle = nullptr;
    }
    return result;
}

/**
 * Note the comment for openRead!
 */
bool PathLockHandler::writeLock() {
    lock_guard<mutex> lock(methodMutex);
    if (this->readLockActive || this->writeLockActive) return false;

    lockedFileHandle = fopen(lockedFile.c_str(), "wb");
    if(lockedFileHandle == nullptr)
        return false;

    bool result = flock(fileno(lockedFileHandle), LOCK_EX | LOCK_NB) == 0;
    if (result)
        this->writeLockActive = result;
    else {
        fclose(lockedFileHandle);
        lockedFileHandle = nullptr;
    }
    return result;
}

bool PathLockHandler::hasLock() {
    return readLockActive || writeLockActive;
}

/**
 * Note the comment for openRead!
 */
void PathLockHandler::unlock() {
    lock_guard<mutex> lock(methodMutex);
    readLockActive = false;
    writeLockActive = false;
    if (this->lockedFileHandle != nullptr) {
        flock(fileno(lockedFileHandle), LOCK_UN);
    }
}
