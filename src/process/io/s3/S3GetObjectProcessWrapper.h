/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3GETOBJECTPROCESSWRAPPER_H
#define FASTQINDEX_S3GETOBJECTPROCESSWRAPPER_H

#include "common/ErrorAccumulator.h"
#include "process/io/s3/S3ServiceOptions.h"
#include <thread>
#include <mutex>

using namespace std;

/**
 * The class is a wrapper for the tool binary "fastqindexs3iohelper".
 * The whole purpose of this is to get a better user experience by swallowing error messages like:
 * - SIGABRT
 * - SIGSEV
 * - SIGPIP
 * all coming up because I did not find (and I searched for it!) a good way to abort a get object request for S3.
 */
class S3GetObjectProcessWrapper : public ErrorAccumulator {
private:

    /**
     * Mutex to perform thread-safe operations in this class
     */
    mutex mtx;

    /**
     * The Thread which is used to run the child S3 process.
     */
    shared_ptr<thread> processThread;

protected:

    /**
     * The options for S3.
     */
    S3ServiceOptions serviceOptions;

    /**
     * The fifo (or file) to write to.
     */
    path fifo;

    /**
     * The object which shall be downloaded.
     */
    string s3Object;

    /**
     * Where to start reading.
     */
    int64_t readStart;

    virtual void processThreadFunc();

public:

    explicit S3GetObjectProcessWrapper(const S3ServiceOptions &serviceOptions,
                                       const path &fifo,
                                       const string &s3Object,
                                       int64_t readStart) {
        this->serviceOptions = serviceOptions;
        this->fifo = fifo;
        this->s3Object = s3Object;
        this->readStart = readStart;
    }

    virtual ~S3GetObjectProcessWrapper() {
        waitForFinish();
        ErrorAccumulator::debug("Abort Thread!");
    }

    /**
     * Start a download process, if no process is already active. Will not stop an active process.
     */
    void start();

    /**
     * Will call join on the underlying thread and thus wait until the download process ends.
     * To really end
     */
    void waitForFinish();
};

typedef shared_ptr<S3GetObjectProcessWrapper> S3GetObjectProcessWrapper_S;

#endif //FASTQINDEX_S3GETOBJECTPROCESSWRAPPER_H
