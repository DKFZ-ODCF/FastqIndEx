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

/**
 * ActualRunner is a base class for Runners which will (actually) perform operations on FASTQ files (index / extract)
 */
class ActualRunner : public Runner {

protected:

    /**
     * The fastq file to work with.
     */
    path fastqFile;

    /**
     * The index file to work with.
     */
    path indexFile;

    ActualRunner(path fastqfile, path indexfile);

public:

    /**
     * Used to check
     * @return
     */
    bool checkPremises() override;

    path getFastqFile() { return fastqFile; }

    path getIndexFile() { return indexFile; }
};


#endif //FASTQINDEX_ACTUALRUNNER_H
