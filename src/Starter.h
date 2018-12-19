/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_STARTER_H
#define FASTQINDEX_STARTER_H

static const char *const FASTQFILE_PARAMETER = "fastqFile";

static const char *const INDEXFILE_PARAMETER = "indexFile";

static const char *const STARTLINE_PARAMETER = "startline";

static const char *const NOOFREADS_PARAMETER = "noofreads";

#include <boost/program_options.hpp>
#include <cstring>
#include "Runner.h"

using namespace boost::program_options;
using namespace std;

class Starter {
private:
    options_description cliOptions;
    options_description hidden;
    positional_options_description posCliOptions;

    static Starter* instance;

public:

    static Starter* getInstance();

    Starter();

    static constexpr const char* INDEX_MODE = "index";

    static constexpr const char* EXTRACTION_MODE = "extract";

    void assembleCLIOptions();

    options_description* getCLIOptions();

    boost::shared_ptr<Runner> createRunner(int argc, const char *argv[]);
};


#endif //FASTQINDEX_STARTER_H
