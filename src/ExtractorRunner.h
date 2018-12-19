/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTORRUNNER_H
#define FASTQINDEX_EXTRACTORRUNNER_H

#include "ActualRunner.h"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

class ExtractorRunner : public ActualRunner {
protected:

    long startLine;

    long lineCount;

public:
    ExtractorRunner(path fastqfile, path indexfile, long startLine, long lineCount);

    bool isExtractor() override { return true; };
};


#endif //FASTQINDEX_EXTRACTORRUNNER_H
