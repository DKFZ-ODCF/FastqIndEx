/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_DONOTHINGRUNNER_H
#define FASTQINDEX_DONOTHINGRUNNER_H

#include "runners/Runner.h"

/**
 * When the application is started, it will create a Starter, which parses the CLI options. The Starter instance will
 * afterwards be used to create a Runner (Extractor, Indexer). If the options are somehow wrong and no valid mode could
 * be determined, a PrintCLIOptionsRunner will be created instead and this will then print out the command line options.
 */
class DoNothingRunner : public Runner {
public :
    DoNothingRunner() = default;

    /**
     * Print CLI options to cerr
     */
    unsigned char _run() override;

    bool isCLIOptionsPrinter() override { return true; }
};


#endif //FASTQINDEX_DONOTHINGRUNNER_H
