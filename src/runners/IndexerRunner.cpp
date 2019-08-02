/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "IndexerRunner.h"
#include "process/index/Indexer.h"

using namespace std;

IndexerRunner::IndexerRunner(
        const shared_ptr<Source> &fastqFile,
        const shared_ptr<Sink> &indexFile,
        shared_ptr<IndexEntryStorageStrategy> storageStrategy,
        bool enableDebugging,
        bool forceOverwrite,
        bool forbidWriteFQI,
        bool compressDictionaries) :
        IndexWritingRunner(fastqFile, indexFile) {
    this->indexer = make_shared<Indexer>(
            this->fastqFile,
            this->indexFile,
            storageStrategy,
            enableDebugging,
            forceOverwrite,
            forbidWriteFQI,
            compressDictionaries
    );
}

IndexerRunner::~IndexerRunner() {
    indexer.reset();
}

bool IndexerRunner::checkPremises() {
    // indexer->checkPremises() will call tryOpenAndReadHeader on the indexer.
    // Errors will be collected.
    bool myPremises = ActualRunner::checkPremises();
    bool indexerPremises = indexer->checkPremises();
    return myPremises && indexerPremises;
}

bool IndexerRunner::allowsReadFromStreamedSource() {
    return true;
}

unsigned char IndexerRunner::_run() {
    if (indexer->createIndex()) return 0;
    else return 1;
}

vector<string> IndexerRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexer->getErrorMessages();
    return mergeToNewVector(l, r);
}
