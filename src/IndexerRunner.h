/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXERRUNNER_H
#define FASTQINDEX_INDEXERRUNNER_H


#include "ActualRunner.h"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

class IndexerRunner : public ActualRunner {

public:
    IndexerRunner(path fastqfile, path indexfile);

    bool isIndexer() override { return true; }

    unsigned char run() override;
};


#endif //FASTQINDEX_INDEXERRUNNER_H
