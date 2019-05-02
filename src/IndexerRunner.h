/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXERRUNNER_H
#define FASTQINDEX_INDEXERRUNNER_H


#include "ActualRunner.h"
#include "Indexer.h"
#include "InputSource.h"
#include "PathInputSource.h"

/**
 * The IndexRunner will, once started, create an index for the  FASTQ file by utilizing the Indexer class. As of now,
 * there is actually no need to separate both classes, as the indexing will always occur with the latest version of the
 * Indexer (there is only one version) whereas the ExtractorRunner, will use an extractor based on the index version.
 * One advantage: The code is a bit more modular and therefore better testable.
 */
class IndexerRunner : public ActualRunner {

private:

    Indexer *indexer = nullptr;

public:

    IndexerRunner(
            const path &fastqfile,
            const path &indexfile,
            int blockInterval = -1,
            bool enableDebugging = false,
            bool forceOverwrite = false
    ) : IndexerRunner(
            shared_ptr<InputSource>(new PathInputSource(fastqfile)),
            indexfile,
            blockInterval,
            enableDebugging,
            forceOverwrite
    ) {};

    IndexerRunner(
            const shared_ptr<InputSource> &fastqfile,
            const path &indexfile,
            int blockInterval = -1,
            bool enableDebugging = false,
            bool forceOverwrite = false
    );

    virtual ~IndexerRunner();

    bool isIndexer() override { return true; }

    bool checkPremises() override;

    /**
     * The Indexer may use a piped input file, thus this will return true for the IndexerRunner!
     */
    bool allowsReadFromStreamedSource() override;

    unsigned char run() override;

    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_INDEXERRUNNER_H
