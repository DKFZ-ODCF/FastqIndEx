/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_ACTUALRUNNER_H
#define FASTQINDEX_ACTUALRUNNER_H

#include "Runner.h"
#include <experimental/filesystem>
#include <string>

using namespace std;
using experimental::filesystem::path;

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

    ActualRunner(const path &fastqfile, const path &indexfile);

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
