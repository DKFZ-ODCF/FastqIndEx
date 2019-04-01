/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "ExtractorRunner.h"
#include "IndexReader.h"
#include "Extractor.h"

ExtractorRunner::ExtractorRunner(const path &fastqfile,
                                 const path &indexfile,
                                 u_int64_t startLine,
                                 u_int64_t lineCount,
                                 bool enableDebugging) :
        ActualRunner(fastqfile, indexfile) {

    this->startLine = startLine;
    this->lineCount = lineCount;
    this->enableDebugging = enableDebugging;
    this->extractor.reset(new Extractor(fastqFile, indexFile, startLine, lineCount, enableDebugging));
}

/**
 * The extractor is a lot more complicated as the indexer, as we might have to deal with several versions of index
 * files. This check will open and close the index file with a basic version of IndexReader. However, the reader
 * will be destroyed and the file will be unlocked! Might be bad.
 */
bool ExtractorRunner::checkPremises() {
    bool baseClassChecksPassed = ActualRunner::checkPremises();
    bool extractorTestsPassed = extractor->checkPremises();
    return baseClassChecksPassed && extractorTestsPassed;
}

unsigned char ExtractorRunner::run() {
    extractor->extractReadsToCout();
}

vector<string> ExtractorRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = extractor->getErrorMessages();
    return mergeToNewVector(l, r);
}
