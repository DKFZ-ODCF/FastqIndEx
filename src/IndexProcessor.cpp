/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <boost/thread/mutex.hpp>
#include "IndexProcessor.h"

IndexProcessor::IndexProcessor(const path &indexFile) : indexFile(indexFile) {
}

const path &IndexProcessor::getIndexFile() const {
    return indexFile;
}

IndexProcessor::~IndexProcessor() {
    unlock();
}

/**
 * The methods look a bit complicated at first glance, but we need some second form of locking
 * when it comes to intra-process locks. It happened in the tests, that the sharable mutex was closed but
 * the write lock could not be established. Using the slightly more complicated syntax below, it works in
 * the tests.
 */
bool IndexProcessor::lockForReading() {
    boost::mutex::scoped_lock sl(methodMutex);
    if (this->writeLockActive) return false;
    bool result = this->interprocessSharableMutex.try_lock_sharable();
    if (result) this->readLockActive = true;
    return result;
}

/**
 * Note the comment for openRead!
 */
bool IndexProcessor::lockForWriting() {
    boost::mutex::scoped_lock sl(methodMutex);
    if (this->readLockActive || this->writeLockActive) return false;
    bool result = this->interprocessSharableMutex.try_lock();
    if (result)
        this->writeLockActive = result;
    return result;
}

/**
 * Note the comment for openRead!
 */
void IndexProcessor::unlock() {
    boost::mutex::scoped_lock sl(methodMutex);
    if (readLockActive)
        this->interprocessSharableMutex.unlock_sharable();
    if (writeLockActive)
        this->interprocessSharableMutex.unlock();
    readLockActive = false;
    writeLockActive = false;
}
