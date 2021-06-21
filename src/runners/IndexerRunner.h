/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXERRUNNER_H
#define FASTQINDEX_INDEXERRUNNER_H


#include "runners/ActualRunner.h"
#include "process/index/Indexer.h"
#include "process/io/Source.h"
#include "process/io/FileSource.h"

/**
 * The IndexRunner will, once started, create an index for the  FASTQ file by utilizing the Indexer class. As of now,
 * there is actually no need to separate both classes, as the indexing will always occur with the latest version of the
 * Indexer (there is only one version) whereas the ExtractorRunner, will use an extractor based on the index version.
 * One advantage: The code is a bit more modular and therefore better testable.
 */
class IndexerRunner : public IndexWritingRunner {

private:

    shared_ptr<Indexer> indexer;

protected:

    unsigned char _run() override;

public:

    IndexerRunner(
            const shared_ptr<Source> &sourceFile,
            const shared_ptr<Sink> &indexFile,
            const shared_ptr<IndexEntryStorageDecisionStrategy>& storageStrategy,
            bool enableDebugging = false,
            bool forceOverwrite = false,
            bool forbidWriteFQI = false,
            bool compressDictionaries = true
    );

    ~IndexerRunner() override;

    bool isIndexer() override { return true; }

    bool fulfillsPremises() override;

    /**
     * The Indexer may use a piped input file, thus this will return true for the IndexerRunner!
     */
    bool allowsReadFromStreamedSource() override;

    vector<string> getErrorMessages() override;

    void enableWritingDecompressedBlocksAndStatistics(const path &location) {
        this->indexer->enableWritingDecompressedBlocksAndStatistics(location);
    }

    void enableWritingPartialDecompressedBlocks(const path &location) {
        this->indexer->enableWritingPartialDecompressedBlocks(location);
    }
};


#endif //FASTQINDEX_INDEXERRUNNER_H
