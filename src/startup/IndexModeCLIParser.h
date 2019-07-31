/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXMODECLIPARSER_H
#define FASTQINDEX_INDEXMODECLIPARSER_H

#include "process/io/Sink.h"
#include "runners/IndexerRunner.h"
#include "startup/ModeCLIParser.h"


class IndexModeCLIParser : public ModeCLIParser {

public:
    IndexerRunner *parse(int argc, const char **argv) override;

    _IntValueArg createBlockIntervalArg(CmdLine *cmdLineParser) const;

    _SwitchArg createDictCompressionSwitchArg(CmdLine *cmdLineParser) const;

    _SwitchArg createForbidIndexWriteoutSwitchArg(CmdLine *cmdLineParser) const;

    _SwitchArg createDisableFailsafeDistanceSwitchArg(CmdLine *cmdLineParser) const;

    _StringValueArg createStoreForPartialDecompressedBlocksArg(CmdLine *cmdLineParser) const;

    _StringValueArg createStoreForDecompressedBlocksArg(CmdLine *cmdLineParser) const;


    static const string descriptForFastqFileArg;
    static const string descriptionForIndexModeIndexFileArg;
    static const string s3ConfigFileSectionArgDescription;
};


#endif //FASTQINDEX_INDEXMODECLIPARSER_H
