/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTORRUNNER_H
#define FASTQINDEX_EXTRACTORRUNNER_H

#include "ActualRunner.h"
#include "Extractor.h"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

/**
 * The ExtractorRunner will, once started, read in the IndexHeader of an FASTQ index file and, based on the encoded
 * indexer version, run the appropriate Extractor version to read in an index file.
 * This modular layout could maybe also be used in another step to convert old indices to new ones.
 */
class ExtractorRunner : public ActualRunner {
protected:

    ulong startLine;

    ulong lineCount;

    bool enableDebugging;

    boost::shared_ptr<Extractor> extractor;

public:
    ExtractorRunner(path fastqfile, path indexfile, ulong startLine, ulong lineCount, bool enableDebugging = false);

    bool isExtractor() override { return true; };

    bool checkPremises() override;

    unsigned char run() override;
};


#endif //FASTQINDEX_EXTRACTORRUNNER_H
