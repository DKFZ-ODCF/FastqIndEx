/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/Source.h"
#include "process/io/PathSource.h"
#include "runners/ExtractorRunner.h"
#include "runners/IndexerRunner.h"
#include "ExtractModeCLIParser.h"
#include "IndexModeCLIParser.h"
#include "IndexStatsModeCLIParser.h"
#include "Starter.h"
#include <tclap/CmdLine.h>

DoNothingRunner *Starter::assembleSmallCmdLineParserAndParseOpts(int argc, const char *argv[]) {
    CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);
    vector<string> allowedValues;
    allowedValues.emplace_back("index");
    allowedValues.emplace_back("extract");
    allowedValues.emplace_back("stats");
    ValuesConstraint<string> allowedModesConstraint(allowedValues);
    UnlabeledValueArg<string> mode("mode", "mode is either index, extract or stats", true, "", &allowedModesConstraint,
                                   cmdLineParser);
    cmdLineParser.parse(argc, argv);
    return new DoNothingRunner();
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
                mode != "stats")
                ) {
            assembleSmallCmdLineParserAndParseOpts(argc, argv);
            return new DoNothingRunner();
        } else if (mode == "index") {
            return assembleCmdLineParserForIndexAndParseOpts(argc, argv);
        } else if (mode == "extract") {
            return assembleCmdLineParserForExtractAndParseOpts(argc, argv);
        } else if (mode == "stats") {
            return assembleCmdLineParserForIndexStatsAndParseOpts(argc, argv);
        }
    } catch (TCLAP::ArgException &e) { // catch any exceptions
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return new DoNothingRunner();
}

shared_ptr<Runner> Starter::createRunner(int argc, const char *argv[]) {
    return shared_ptr<Runner>(assembleCLIOptions(argc, argv));
}
