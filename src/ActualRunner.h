/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_ACTUALRUNNER_H
#define FASTQINDEX_ACTUALRUNNER_H

#include <string>
#include "Runner.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

class ActualRunner : public Runner {

protected:

    path fastqFile;

    path indexFile;

    ActualRunner(path fastqfile, path indexfile);

public:

    bool checkPremises() override;

    path getFastqFile() { return fastqFile; }

    path getIndexFile() { return indexFile; }
};


#endif //FASTQINDEX_ACTUALRUNNER_H
