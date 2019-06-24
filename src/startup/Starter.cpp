/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../runners/ExtractorRunner.h"
#include "../runners/IndexerRunner.h"
#include "../process/io/InputSource.h"
#include "../process/io/PathInputSource.h"
#include "../process/io/StreamInputSource.h"
#include "ExtractModeCLIParser.h"
#include "IndexModeCLIParser.h"
#include "Starter.h"
#include "ModeCLIParser.h"
#include <cstring>
#include <tclap/CmdLine.h>

Starter *Starter::instance = nullptr;

Starter *Starter::getInstance() {
    mutex lock;
    lock.lock();
    if (!Starter::instance) Starter::instance = new Starter();
    lock.unlock();
    return Starter::instance;
}

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
    CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);
    ValueArg<string> indexFile(
            "i", "indexfile",
            string("The index file which shall be used for extraction. If the value is not set, .fqi will be") +
            "to the name of the FASTQ file.",
            false,
            "", "string", cmdLineParser);

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

    vector<string> allowedMode{"stats"};
    ValuesConstraint<string> allowedModesConstraint(allowedMode);
    UnlabeledValueArg<string> mode("mode", "mode is stats", true, "", &allowedModesConstraint);
    cmdLineParser.add(mode);

    cmdLineParser.parse(argc, argv);

    path index(indexFile.getValue());
    return new IndexStatsRunner(index, startingread.getValue(), numberofreads.getValue());
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
}

shared_ptr<Runner> Starter::createRunner(int argc, const char *argv[]) {
    return shared_ptr<Runner>(assembleCLIOptions(argc, argv));
}
