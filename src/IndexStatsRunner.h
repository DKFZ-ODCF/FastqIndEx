/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXSTATSRUNNER_H
#define FASTQINDEX_INDEXSTATSRUNNER_H


#include "ActualRunner.h"
#include "IndexReader.h"

class IndexStatsRunner : public ActualRunner {

private:

    shared_ptr<IndexReader> indexReader;

    int start;

    int amount;

public:

    IndexStatsRunner(const path &indexfile, int start, int amount);

    bool allowsReadFromStreamedSource() override;

    bool checkPremises() override;

    unsigned char run() override;

    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_INDEXSTATSRUNNER_H
