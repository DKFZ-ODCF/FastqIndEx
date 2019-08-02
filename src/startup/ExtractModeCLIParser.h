/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTMODECLIPARSER_H
#define FASTQINDEX_EXTRACTMODECLIPARSER_H

#include "runners/ExtractorRunner.h"
#include "ModeCLIParser.h"

class ExtractModeCLIParser : public ModeCLIParser {

public:
    ExtractorRunner *parse(int arc, const char **argv) override;

    _UIntValueArg createSegmentCountArg(CmdLine *cmdLineParser) const;

    _UIntValueArg createSegmentIdentifierArg(CmdLine *cmdLineParser) const;

    _UInt64ValueArg createNumberOfReadsArg(CmdLine *cmdLineParser) const;

    _UInt64ValueArg createStartingReadArg(CmdLine *cmdLineParser) const;

    _IntValueArg createExtractionMultiplierArg(CmdLine *cmdLineParser) const;

    _StringValueArg createOutputFileArg(CmdLine *cmdLineParser) const;

};


#endif //FASTQINDEX_EXTRACTMODECLIPARSER_H
