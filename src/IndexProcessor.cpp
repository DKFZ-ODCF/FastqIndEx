/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */
#include "IndexProcessor.h"
#include <mutex>
#include <sys/file.h>
#include <iostream>
#include <utility>
using std::lock_guard;
using std::mutex;

IndexProcessor::IndexProcessor(path indexFile)  {
    this->indexFile = std::move(indexFile);
}

const path &IndexProcessor::getIndexFile() const {
    return indexFile;
}

IndexProcessor::~IndexProcessor() {
    unlock();
}

/**
 * The methods looks a bit complicated at first glance, but we need some second form of locking
 * when it comes to intra-process locks. It happened in the tests, that the sharable mutex was closed but
 * the write lock could not be established. Using the slightly more complicated syntax below, it works in
 * the tests.
 */
bool IndexProcessor::openWithReadLock() {
    lock_guard<mutex> lock(methodMutex);
    if (this->readLockActive)
        return true;
    indexFileHandle = fopen(indexFile.c_str(), "rb");
    bool result = flock(fileno(indexFileHandle), LOCK_SH | LOCK_NB) == 0;
    if (result)
        this->readLockActive = true;
    else {
        fclose(indexFileHandle);
        indexFileHandle == nullptr;
    }
    return result;
}

/**
 * Note the comment for openRead!
 */
bool IndexProcessor::openWithWriteLock() {
    lock_guard<mutex> lock(methodMutex);
    if (this->readLockActive || this->writeLockActive) return false;

    indexFileHandle = fopen(indexFile.c_str(), "wb");
    if(indexFileHandle == nullptr)
        return false;

    bool result = flock(fileno(indexFileHandle), LOCK_EX | LOCK_NB) == 0;
    if (result)
        this->writeLockActive = result;
    else {
        fclose(indexFileHandle);
        indexFileHandle == nullptr;
    }
    return result;
}

bool IndexProcessor::hasLock() {
    return readLockActive || writeLockActive;
}

/**
 * Note the comment for openRead!
 */
void IndexProcessor::unlock() {
    lock_guard<mutex> lock(methodMutex);
    readLockActive = false;
    writeLockActive = false;
    if (this->indexFileHandle != nullptr) {
        flock(fileno(indexFileHandle), LOCK_UN);
    }
}
