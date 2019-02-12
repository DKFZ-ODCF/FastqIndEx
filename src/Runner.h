/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_RUNNER_H
#define FASTQINDEX_RUNNER_H

#include <string>
#include <vector>
#include "ErrorAccumulator.h"

using namespace std;

/**
 * When the application is started it will first create a Starter object which will parse CLI options and finally create
 * a Runner instance. The Runner instance will afterwards be used to run the application logic (index, extract or print
 * cli opts).
 */
class Runner : public ErrorAccumulator {

protected:
    Runner() = default;

public:

    /**
     * Can return an exit code between 0 and 255.
     */
    virtual unsigned char run() {};

    /**
     * Check premises for the runner instance and accumulate errors, if the checks fail.
     * @return true if the premises were validated or false.
     */
    virtual bool checkPremises() { return true; };

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

/**
 * When the application is started, it will create a Starter, which parses the CLI options. The Starter instance will
 * afterwards be used to create a Runner (Extractor, Indexer). If the options are somehow wrong and no valid mode could
 * be determined, a PrintCLIOptionsRunner will be created instead and this will then print out the command line options.
 */
class PrintCLIOptionsRunner : public Runner {
public :
    PrintCLIOptionsRunner() = default;

    /**
     * Print CLI options to cerr
     */
    unsigned char run() override;

    bool isCLIOptionsPrinter() override { return true; }
};





#endif //FASTQINDEX_RUNNER_H
