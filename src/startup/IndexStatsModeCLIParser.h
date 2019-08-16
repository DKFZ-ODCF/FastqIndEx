/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXSTATSMODECLIPARSER_H
#define FASTQINDEX_INDEXSTATSMODECLIPARSER_H

#include "process/io/s3/S3Source.h"
#include "Starter.h"
#include "ModeCLIParser.h"

class IndexStatsModeCLIParser : public ModeCLIParser {
public:
    IndexStatsRunner *parse(int argc, const char **argv) override {
        auto cmdLineParser = createCommandLineParser();

        auto startingReadArg = _makeUIntValueArg(
                "s", "startingentry",
                "Defines the first index entry to show.",
                false,
                0, cmdLineParser.get());

        auto numberOfReadsArg = _makeUIntValueArg(
                "n", "numberofentries",
                "Number of index entries to show.",
                false,
                1, cmdLineParser.get());

        auto s3ConfigFileSectionArg = createS3ConfigFileSectionArg(cmdLineParser.get());
        auto s3CredentialsFileArg = createS3CredentialsFileArg(cmdLineParser.get());
        auto s3ConfigFileArg = createS3ConfigFileArg(cmdLineParser.get());

        auto indexFileArg = createIndexFileArg(cmdLineParser.get());

        // Keep the mode constraints on the stack, so allowedModeArg won't access invalid memory!
        auto[allowedModeArg, modeConstraints] = createAllowedModeArg("stats", cmdLineParser.get());

        cmdLineParser->parse(argc, argv);

        S3ServiceOptions s3ServiceOptions(s3ConfigFileArg->getValue(),
                                          s3CredentialsFileArg->getValue(),
                                          s3ConfigFileSectionArg->getValue());
        auto s3Service = S3Service::from(s3ServiceOptions);

        auto index = processIndexFileSource(indexFileArg->getValue(), s3Service);

        auto runner = new IndexStatsRunner(index, startingReadArg->getValue(), numberOfReadsArg->getValue());

        return runner;
    }
};

#endif //FASTQINDEX_INDEXSTATSMODECLIPARSER_H
