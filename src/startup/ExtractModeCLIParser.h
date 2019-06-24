/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTMODECLIPARSER_H
#define FASTQINDEX_EXTRACTMODECLIPARSER_H

#include "../runners/ExtractorRunner.h"
#include "ModeCLIParser.h"

class ExtractModeCLIParser : public ModeCLIParser {

public:
    ExtractorRunner *parse(int arc, const char **argv) override;

};


#endif //FASTQINDEX_EXTRACTMODECLIPARSER_H
