/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "ExtractorRunner.h"

ExtractorRunner::ExtractorRunner(path fastqfile, path indexfile, long startLine, long lineCount) :
        ActualRunner(fastqfile, indexfile) {

    this->startLine = startLine;
    this->lineCount = lineCount;
}