/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXERRUNNER_H
#define FASTQINDEX_INDEXERRUNNER_H


#include "ActualRunner.h"
#include "Indexer.h"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

/**
 * The IndexRunner will, once started, create an index for the  FASTQ file by utilizing the Indexer class. As of now,
 * there is actually no need to separate both classes, as the indexing will always occur with the latest version of the
 * Indexer (there is only one version) whereas the ExtractorRunner, will use an extractor based on the index version.
 * One advantage: The code is a bit more modular and therefore better testable.
 */
class IndexerRunner : public ActualRunner {

private:

    boost::shared_ptr<Indexer> indexer = boost::shared_ptr<Indexer>(nullptr);

public:

    IndexerRunner(path fastqfile, path indexfile, bool enableDebugging = false);

    bool isIndexer() override { return true; }

    bool checkPremises() override;

    unsigned char run() override;

    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_INDEXERRUNNER_H
