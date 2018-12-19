/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "IndexerRunner.h"
#include "Indexer.h"

using namespace std;

IndexerRunner::IndexerRunner(path fastqfile, path indexfile) :
        ActualRunner(fastqfile, indexfile) {
}

unsigned char IndexerRunner::run() {

    Indexer indexer(this->fastqFile, this->indexFile);

    indexer.createIndex();
}
