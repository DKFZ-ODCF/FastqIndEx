/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXPROCESSOR_H
#define FASTQINDEX_INDEXPROCESSOR_H

#include "../../common/ErrorAccumulator.h"
#include <mutex>
#include <shared_mutex>
#include <experimental/filesystem>

using namespace std;
using std::experimental::filesystem::path;
using std::lock_guard;
using std::mutex;


/**
 * Base class for IndexReader and IndexWriter.
 * Handles interprocess locking for the input/output file.
 * Access to the index file is mutually exclusive!
 * Reading from or writing to index files never happens with a pipe as index files are really small.
 *
 * @see https://stackoverflow.com/questions/12439099/interprocess-reader-writer-lock-with-boost
 */
class IndexProcessor : public ErrorAccumulator {

private:

    /**
     * Better play safe in methods handling the above mutex.
     * Testing this would be cumbersome, we assume that the boost developers did their job and that it works as
     * expected.
     */
    mutex methodMutex;

    bool readLockActive = false;

    bool writeLockActive = false;

    FILE* indexFileHandle = nullptr;

protected:

    /**
     * The original path of the index file.
     */
    path indexFile;

public:

    explicit IndexProcessor(path indexFile);

    ~IndexProcessor();

    const path &getIndexFile() const;

    /**
     * Use this to open the output file.
     * This will use an interprocess mutex to ensure, that file operations on the index file are safe!
     * @return
     */
    bool openWithReadLock();

    /**
     * Use this to open the output file.
     * This will use an interprocess mutex to ensure, that file operations on the index file are safe!
     * @return
     */
    bool openWithWriteLock();

    /**
     * @return true, if the object has a read or write lock otherwise false.
     */
    bool hasLock();

    /**
     * Use this to close the output file.
     * This will use an interprocess mutex to ensure, that file operations on the index file are safe!
     */
    void unlock();
};

#endif //FASTQINDEX_INDEXPROCESSOR_H
