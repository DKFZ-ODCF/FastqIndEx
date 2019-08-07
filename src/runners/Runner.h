/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_RUNNER_H
#define FASTQINDEX_RUNNER_H

#include "common/ErrorAccumulator.h"
#include "process/io/s3/S3Service.h"
#include <string>
#include <vector>

using namespace std;

/**
 * When the application is started it will first create a Starter object which will parse CLI options and finally create
 * a Runner instance. The Runner instance will afterwards be used to run the application logic (index, extract or print
 * cli opts).
 */
class Runner : public ErrorAccumulator {

private:

    S3ServiceOptions s3Options;

    bool s3Enabled{false};

protected:
    Runner() = default;

    virtual unsigned char _run() { return 0; };

public:

    virtual ~Runner() = default;

    /**
     * Can return an exit code between 0 and 255.
     */
    unsigned char run();

    /**
     * Check premises for the runner instance and accumulate errors, if the checks fail.
     * @return true if the premises were validated or false.
     */
    virtual bool fulfillsPremises() { return true; };

    /**
     * Actually for debugging.
     */
    virtual bool isCLIOptionsPrinter() { return false; };

    /**
     * Actually for debugging.
     */
    virtual bool isIndexer() { return false; };

    /**
     * Actually for debugging.
     */
    virtual bool isExtractor() { return false; };
};


#endif //FASTQINDEX_RUNNER_H
