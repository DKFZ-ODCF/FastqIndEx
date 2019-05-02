/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "IndexerRunner.h"
#include "Indexer.h"

using namespace std;

IndexerRunner::IndexerRunner(
        const shared_ptr<InputSource> &fastqfile,
        const path &indexfile,
        int blockInterval,
        bool enableDebugging,
        bool forceOverwrite
) :
        ActualRunner(fastqfile, indexfile) {
    this->indexer = new Indexer(this->fastqFile, this->indexFile, blockInterval, enableDebugging, forceOverwrite);
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

bool IndexerRunner::allowsReadFromStreamedSource() {
    return true;
}

unsigned char IndexerRunner::run() {
    if (indexer->createIndex()) return 0;
    else return 1;
}

vector<string> IndexerRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexer->getErrorMessages();
    return mergeToNewVector(l, r);
}
