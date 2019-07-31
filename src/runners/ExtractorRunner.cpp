/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "ExtractorRunner.h"
#include "process/extract/IndexReader.h"
#include "process/extract/Extractor.h"
#include "process/io/PathSource.h"

ExtractorRunner::ExtractorRunner(
        const shared_ptr<Source> &fastqfile,
        const shared_ptr<Source> &indexFile,
        const shared_ptr<Sink> &resultfile,
        bool forceOverwrite,
        ExtractMode mode,
        u_int64_t start,
        u_int64_t count,
        uint extractionMultiplier,
        bool enableDebugging
) : IndexReadingRunner(fastqfile, indexFile) {

    this->start = start;
    this->count = count;
    this->extractionMultiplier = extractionMultiplier;
    this->enableDebugging = enableDebugging;
    this->extractor.reset(
            new Extractor(fastqfile, indexFile, resultfile, forceOverwrite, mode, start, count,
                          extractionMultiplier, enableDebugging)
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

unsigned char ExtractorRunner::_run() {
    return extractor->extract() ? 0 : 1;
}

vector<string> ExtractorRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = extractor->getErrorMessages();
    return mergeToNewVector(l, r);
}
