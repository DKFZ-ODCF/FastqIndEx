/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "Indexer.h"

const unsigned int Indexer::CHUNK_SIZE = 16 * 1024;

const unsigned int Indexer::WINDOW_SIZE = 128 * 1024;

const unsigned int Indexer::INDEXER_VERSION = 1;

Indexer::Indexer(const path &fastq, const path &index, bool enableDebugging) :
        fastq(fastq),
        index(index),
        debuggingEnabled(enableDebugging) {
    storedHeader = boost::shared_ptr<IndexHeader>(nullptr);
}

boost::shared_ptr<IndexHeader> Indexer::createHeader() {
    return boost::make_shared<IndexHeader>(Indexer::INDEXER_VERSION);
}

bool Indexer::createIndex() {

    auto header = createHeader();

    if(debuggingEnabled)
        storedHeader = header;

    finishedSuccessful = true;

    return finishedSuccessful;
}
