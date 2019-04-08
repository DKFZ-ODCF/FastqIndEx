/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_STARTER_H
#define FASTQINDEX_STARTER_H

#include "Runner.h"
#include "IndexerRunner.h"
#include "ExtractorRunner.h"
#include <tclap/CmdLine.h>
#include <cstring>
#include <memory>

using namespace std;
using namespace TCLAP;

static const char *const FASTQFILE_PARAMETER = "fastqFile";

static const char *const INDEXFILE_PARAMETER = "indexFile";

static const char *const STARTLINE_PARAMETER = "startline";

static const char *const NOOFREADS_PARAMETER = "noofreads";

/**
 * The starter is the entrypoint for our application. It will take the command line arguments and try to transform them
 * to an object of the Runner type. If the options could not be recognized, an object of type PrintCLIOptionsRunner will
 * be created and used to display all command line options.
 * The starter is a singleton and will be created and destroyed in the main method.
 */
class Starter {
private:

    static Starter *instance;

public:

    static Starter *getInstance();

    Starter() = default;

    path argumentToPath(ValueArg<string> &cliArg) const;

    DoNothingRunner* assembleSmallCmdLineParserAndParseOpts(int argc, const char **argv);

    IndexerRunner* assembleCmdLineParserForIndexAndParseOpts(int argc, const char **argv);

    ExtractorRunner* assembleCmdLineParserForExtractAndParseOpts(int argc, const char **argv);

    Runner* assembleCLIOptions(int argc, const char *argv[]);

    shared_ptr<Runner> createRunner(int argc, const char *argv[]);

};


#endif //FASTQINDEX_STARTER_H
