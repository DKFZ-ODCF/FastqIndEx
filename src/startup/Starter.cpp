/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/Source.h"
#include "process/io/FileSource.h"
#include "runners/ExtractorRunner.h"
#include "runners/IndexerRunner.h"
#include "startup/ExtractModeCLIParser.h"
#include "startup/IndexModeCLIParser.h"
#include "startup/IndexStatsModeCLIParser.h"
#include "startup/Starter.h"
#include "startup/S3TestModeCLIParser.h"
#include <tclap/CmdLine.h>

PrintCLIOptions *Starter::assembleSmallCmdLineParserAndParseOpts(int argc, const char *argv[]) {
    CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);
    vector<string> allowedValues;
    allowedValues.emplace_back("index");
    allowedValues.emplace_back("extract");
    allowedValues.emplace_back("stats");
    allowedValues.emplace_back("tests3");
    ValuesConstraint<string> allowedModesConstraint(allowedValues);
    UnlabeledValueArg<string> mode("mode", "mode is either index, extract, stats or tests3", true, "", &allowedModesConstraint,
                                   cmdLineParser);
    cmdLineParser.parse(argc, argv);
    return new PrintCLIOptions();
}

IndexStatsRunner *Starter::assembleCmdLineParserForIndexStatsAndParseOpts(int argc, const char **argv) {
    return IndexStatsModeCLIParser().parse(argc, argv);
}

IndexerRunner *Starter::assembleCmdLineParserForIndexAndParseOpts(int argc, const char **argv) {
    return IndexModeCLIParser().parse(argc, argv);
}

ExtractorRunner *Starter::assembleCmdLineParserForExtractAndParseOpts(int argc, const char **argv) {
    return ExtractModeCLIParser().parse(argc, argv);
}

S3TestRunner *Starter::assembleCmdLineParserForTestS3AndParseOpts(int argc, const char **argv) {
    return S3TestModeCLIParser().parse(argc, argv);
}

/**
 * Effectively checks parameter count and file existence and accessibility
 * @param argc parameter count
 * @param argv parameter array
 */
Runner *Starter::assembleCLIOptions(int argc, const char *argv[]) {
    try {
        string mode;
        if (argc > 1)
            mode = string(argv[1]);

        // Neither index nor extract provided
        if (argc == 1 || (
                mode != "index" &&
                mode != "extract" &&
                mode != "stats" &&
                mode != "tests3")
                ) {
            assembleSmallCmdLineParserAndParseOpts(argc, argv);
            return new PrintCLIOptions();
        } else if (mode == "index") {
            return assembleCmdLineParserForIndexAndParseOpts(argc, argv);
        } else if (mode == "extract") {
            return assembleCmdLineParserForExtractAndParseOpts(argc, argv);
        } else if (mode == "stats") {
            return assembleCmdLineParserForIndexStatsAndParseOpts(argc, argv);
        } else if (mode == "tests3") {
            return assembleCmdLineParserForTestS3AndParseOpts(argc, argv);
        }
    } catch (TCLAP::ArgException &e) { // catch any exceptions
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return new PrintCLIOptions();
}

shared_ptr<Runner> Starter::createRunner(int argc, const char *argv[]) {
    return shared_ptr<Runner>(assembleCLIOptions(argc, argv));
}
