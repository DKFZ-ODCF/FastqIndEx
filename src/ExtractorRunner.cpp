/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "ExtractorRunner.h"
#include "IndexReader.h"
#include "Extractor.h"
#include "PathInputSource.h"

ExtractorRunner::ExtractorRunner(
        const shared_ptr<PathInputSource> &fastqfile,
        const path &indexfile,
        const path &resultfile,
        bool forceOverwrite,
        u_int64_t startLine,
        u_int64_t lineCount,
        uint extractionMulitplier,
        bool enableDebugging
) : ActualRunner(fastqfile, indexfile) {

    this->startLine = startLine;
    this->lineCount = lineCount;
    this->extractionMulitplier = extractionMulitplier;
    this->enableDebugging = enableDebugging;
    this->extractor.reset(
            new Extractor(fastqfile, indexfile, resultfile, forceOverwrite, startLine, lineCount, extractionMulitplier, enableDebugging)
    );
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
    return extractor->extract() ? 0 : 1;
}

vector<string> ExtractorRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = extractor->getErrorMessages();
    return mergeToNewVector(l, r);
}
