/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Starter.h"
#include "IndexerRunner.h"
#include "ExtractorRunner.h"
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

path Starter::argumentToPath(ValueArg<string> &cliArg) const {
    path _path;
    if (cliArg.getValue() == "-")
        _path = path("/-");
    else
        _path = path(cliArg.getValue());
    return _path;
}

DoNothingRunner *Starter::assembleSmallCmdLineParserAndParseOpts(int argc, const char *argv[]) {
    CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);
    vector<string> allowedValues;
    allowedValues.emplace_back("index");
    allowedValues.emplace_back("extract");
    ValuesConstraint<string> allowedModesConstraint(allowedValues);
    UnlabeledValueArg<string> mode("mode", "mode is either index or extract", true, "", &allowedModesConstraint,
                                   cmdLineParser);
    cmdLineParser.parse(argc, argv);
    return new DoNothingRunner();
}

IndexerRunner *Starter::assembleCmdLineParserForIndexAndParseOpts(int argc, const char **argv) {
    CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);


    ValueArg<string> indexFile(
            "i",
            "indexfile",
            string("The index file which shall be created or - for stdout or \"\" to append .idx to the FASTQ filename. ") +
            "Note, that the index will be streamed to stdout, if you provide \"\" or - as the FASTQ file parameter.",
            false,
            "",
            "string", cmdLineParser);

    ValueArg<string> fastqFile(
            "f",
            "fastqfile",
            "The fastq file which shall be indexed or - for stdin.",
            true,
            "-",
            "string", cmdLineParser);

    ValueArg<int> blockInterval(
            "b",
            "blockinterval",
            "Defines, that an index entry will be written to the index file for every nth compressed block. Value needs to be in range from [1 .. n].",
            false,
            -1,
            "int", cmdLineParser);

    vector<string> allowedMode{"index"};
    ValuesConstraint<string> allowedModesConstraint(allowedMode);
    UnlabeledValueArg<string> mode("mode", "mode is index", true, "", &allowedModesConstraint);
    cmdLineParser.add(mode);

    cmdLineParser.parse(argc, argv);

    path fastq = argumentToPath(fastqFile);
    path index = argumentToPath(indexFile);

    if (fastqFile.getValue().empty()) {
        fastq = "/-";  // Set to stdin
    }

    if (indexFile.getValue().empty()) {
        if (fastq == "/-") {
            index = "/-";  // Set to stdout
        } else {
            index = fastq.string() + ".idx";
        }
    }

    int bi = blockInterval.getValue();
    if (bi < -1) bi = -1;

    return new IndexerRunner(fastq, index, bi);
}

ExtractorRunner *Starter::assembleCmdLineParserForExtractAndParseOpts(int argc, const char **argv) {
    CmdLine cmdLineParser("Extract reads from a FASTQ file (or lines from a text file).", '=', "0.0.1", false);

    vector<string> allowedMode{"extract"};
    ValuesConstraint<string> allowedModesConstraint(allowedMode);
    UnlabeledValueArg<string> mode("mode", "mode is extract", true, "", &allowedModesConstraint);
    cmdLineParser.add(mode);

    ValueArg<string> outFile(
            "o",
            "outfile",
            "The output file which shall be created or - (default) for stdout.",
            false,
            "-",
            "string", cmdLineParser);

    ValueArg<string> indexFile(
            "i",
            "indexfile",
            string("The index file which shall be used for extraction. If the value is not set, .idx will be") +
            "to the name of the FASTQ file.",
            false,
            "",
            "string", cmdLineParser);

    ValueArg<string> fastqFile(
            "f",
            "fastqfile",
            "The FASTQ (or text) file from which shall be extracted.",
            true,
            "",
            "string", cmdLineParser);

    ValueArg<int> extractionmultiplier(
            "e",
            "extractionmultiplier",
            string("Defines a multiplier by which the startingline parameter will be mulitplied. For FASTQ files ") +
            "this is 4 (record size), but you could use 1 for e.g. regular text files.",
            false,
            4,
            "int", cmdLineParser);

//    SwitchArg disablefastqchecks(
//            "d",
//            "disablefastqchecks",
//            "Will (in the future) disable further checks for FASTQ consistency.",
//            false);
//    cmdLineParser.add(disablefastqchecks);

    ValueArg<int> startingread(
            "s",
            "startingread",
            "Defines the first line (multiplied by extractionmultiplier) which should be extracted.",
            false,
            0,
            "int", cmdLineParser);

    ValueArg<int> numberofreads(
            "n",
            "numberofreads",
            "Defines the number of reads which should be extracted. The size of each read is defined by extractionmultiplier.",
            false,
            10,
            "int", cmdLineParser);

    cmdLineParser.parse(argc, argv);
    return new ExtractorRunner(
            argumentToPath(fastqFile),
            argumentToPath(indexFile),
            startingread.getValue() * extractionmultiplier.getValue(),
            numberofreads.getValue() * extractionmultiplier.getValue()
    );
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
        if (argc == 1 ||
            (mode != "index" && mode != "extract")) {
            assembleSmallCmdLineParserAndParseOpts(argc, argv);
            return new DoNothingRunner();
        } else if (mode == "index") {
            return assembleCmdLineParserForIndexAndParseOpts(argc, argv);
        } else if (mode == "extract") {
            return assembleCmdLineParserForExtractAndParseOpts(argc, argv);
        }
    } catch (TCLAP::ArgException &e) { // catch any exceptions
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}

shared_ptr<Runner> Starter::createRunner(int argc, const char *argv[]) {
    return shared_ptr<Runner>(assembleCLIOptions(argc, argv));
}