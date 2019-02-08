/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXPROCESSOR_H
#define FASTQINDEX_INDEXPROCESSOR_H

#include "ErrorAccumulator.h"
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/thread/mutex.hpp>

using namespace boost;
using namespace boost::filesystem;
using namespace boost::interprocess;

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
     * Interprocess lock which grants shared reader and exclusive writer access to the index file.
     * The lock stays open as long as either close() was called OR the IndexProcessor instance is destroyed.
     */
    boost::interprocess::interprocess_sharable_mutex interprocessSharableMutex;

    /**
     * Better play safe in methods handling the above mutex.
     * Testing this would be cumbersome, we assume that the boost developers did their job and that it works as
     * expected.
     */
    boost::mutex methodMutex;

    bool readLockActive = false;

    bool writeLockActive = false;


protected:

    path indexFile;

public:

    explicit IndexProcessor(const path &indexFile);

    ~IndexProcessor();

    const path &getIndexFile() const;

    /**
     * Use this to open the output file.
     * This will use an interprocess mutex to ensure, that file operations on the index file are safe!
     * @return
     */
    bool lockForReading();

    /**
     * Use this to open the output file.
     * This will use an interprocess mutex to ensure, that file operations on the index file are safe!
     * @return
     */
    bool lockForWriting();

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
