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
        CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);

        auto indexFileArg = createIndexFileArg(&cmdLineParser);

        ValueArg<int> startingread(
                "s", "startingentry",
                "Defines the first to show.",
                false,
                0, "int", cmdLineParser);

        ValueArg<int> numberofreads(
                "n", "numberofentries",
                "Number of index entries to show.",
                false,
                1, "int", cmdLineParser);

        auto s3ConfigFileArg = createS3ConfigFileArg(&cmdLineParser);

        auto s3CredentialsFileArg = createS3CredentialsFileArg(&cmdLineParser);

        auto s3ConfigFileSectionArg = createS3ConfigFileSectionArg(&cmdLineParser);

        vector<string> allowedMode{"stats"};
        ValuesConstraint<string> allowedModesConstraint(allowedMode);
        UnlabeledValueArg<string> mode("mode", "mode is stats", true, "", &allowedModesConstraint);
        cmdLineParser.add(mode);

        cmdLineParser.parse(argc, argv);

        S3ServiceOptions s3ServiceOptions(s3ConfigFileArg->getValue(),
                                          s3CredentialsFileArg->getValue(),
                                          s3ConfigFileSectionArg->getValue());
        S3Service::setS3ServiceOptions(s3ServiceOptions);

        auto index = processIndexFileSource(indexFileArg->getValue(), s3ServiceOptions);

        auto runner = new IndexStatsRunner(index, startingread.getValue(), numberofreads.getValue());

        return runner;
    }
};

#endif //FASTQINDEX_INDEXSTATSMODECLIPARSER_H
