/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3TESTMODECLIPARSER_H
#define FASTQINDEX_S3TESTMODECLIPARSER_H

#include "process/io/s3/S3Source.h"
#include "runners/S3TestRunner.h"
#include "Starter.h"
#include "ModeCLIParser.h"

class S3TestModeCLIParser : public ModeCLIParser {
public:
    S3TestRunner *parse(int argc, const char **argv) override {
        auto cmdLineParser = createCommandLineParser();

        auto bucketArg = _makeStringValueArg(
                "b", "bucket",
                "The S3 bucket name (without the s3:// prefix).",
                true, "",
                cmdLineParser.get()
        );

        auto s3ConfigFileSectionArg = createS3ConfigFileSectionArg(cmdLineParser.get());
        auto s3CredentialsFileArg = createS3CredentialsFileArg(cmdLineParser.get());
        auto s3ConfigFileArg = createS3ConfigFileArg(cmdLineParser.get());

        // Keep the mode constraints on the stack, so allowedModeArg won't access invalid memory!
        auto[allowedModeArg, modeConstraints] = createAllowedModeArg("tests3", cmdLineParser.get());

        cmdLineParser->parse(argc, argv);

        S3ServiceOptions s3ServiceOptions(s3ConfigFileArg->getValue(),
                                          s3CredentialsFileArg->getValue(),
                                          s3ConfigFileSectionArg->getValue());
        auto s3Service = S3Service::from(s3ServiceOptions);

        return new S3TestRunner(s3Service, bucketArg->getValue());
    }
};

#endif //FASTQINDEX_S3TESTMODECLIPARSER_H
