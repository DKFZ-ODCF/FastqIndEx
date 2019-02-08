/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "IndexerRunner.h"
#include "Indexer.h"

using namespace std;

IndexerRunner::IndexerRunner(path fastqfile, path indexfile, bool enableDebugging) :
        ActualRunner(fastqfile, indexfile) {

    this->indexer.reset(new Indexer(this->fastqFile, this->indexFile, enableDebugging));
}

bool IndexerRunner::checkPremises() {
    // indexer->checkPremises() will call tryOpen on the indexer.
    // Errors will be collected.
    return ActualRunner::checkPremises() && indexer->checkPremises();
}

unsigned char IndexerRunner::run() {
    indexer->createIndex();
}

vector<string> IndexerRunner::getErrorMessages() {
    return mergeToNewVector(ErrorAccumulator::getErrorMessages(), indexer->getErrorMessages());
}
