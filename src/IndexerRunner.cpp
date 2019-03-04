/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "IndexerRunner.h"
#include "Indexer.h"

using namespace std;

IndexerRunner::IndexerRunner(path fastqfile, path indexfile, bool enableDebugging) :
        ActualRunner(fastqfile, indexfile) {
    this->indexer = new Indexer(this->fastqFile, this->indexFile, -1, enableDebugging);
}

IndexerRunner::~IndexerRunner() {
    delete indexer;
}

bool IndexerRunner::checkPremises() {
    // indexer->checkPremises() will call tryOpenAndReadHeader on the indexer.
    // Errors will be collected.
    bool myPremises = ActualRunner::checkPremises();
    bool indexerPremises = indexer->checkPremises();
    return myPremises && indexerPremises;
}

unsigned char IndexerRunner::run() {
    indexer->createIndex();
}

vector<string> IndexerRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexer->getErrorMessages();
    return mergeToNewVector(l, r);
}
