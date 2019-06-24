/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXMODECLIPARSER_H
#define FASTQINDEX_INDEXMODECLIPARSER_H

#include "../runners/IndexerRunner.h"
#include "ModeCLIParser.h"

class IndexModeCLIParser : public ModeCLIParser {

public:
    IndexerRunner *parse(int argc, const char **argv) override;

};


#endif //FASTQINDEX_INDEXMODECLIPARSER_H
