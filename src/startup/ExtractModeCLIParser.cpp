/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "ExtractModeCLIParser.h"

#include <tclap/CmdLine.h>

using namespace std;
using namespace TCLAP;

ExtractorRunner* ExtractModeCLIParser::parse(int argc, const char** argv) {
    CmdLine cmdLineParser("Extract reads from a FASTQ file (or lines from a text file).", '=', "0.0.1", false);

    vector<string> allowedMode{"extract"};
    ValuesConstraint<string> allowedModesConstraint(allowedMode);
    UnlabeledValueArg<string> mode("mode", "mode is extract", true, "", &allowedModesConstraint);
    cmdLineParser.add(mode);

    ValueArg<string> outFile(
            "o", "outfile",
            "The uncompressed output FASTQ or textfile which shall be created or - (default) for stdout.",
            false,
            "-", "string", cmdLineParser);

    ValueArg<string> indexFile(
            "i", "indexfile",
            string("The index file which shall be used for extraction. If the value is not set, .fqi will be") +
            "appended to the name of the FASTQ file.",
            false,
            "", "string", cmdLineParser);

    ValueArg<string> fastqFile(
            "f", "fastqfile",
            "The FASTQ (or text) file from which shall be extracted.",
            true,
            "", "string", cmdLineParser);

    SwitchArg forceOverwrite(
            "w", "forceoverwrite",
            "Allow the indexer to overwrite an existing index file.",
            cmdLineParser);

    ValueArg<int> extractionmultiplier(
            "e", "extractionmultiplier",
            string("Defines a multiplier by which the startingline parameter will be multiplied. For FASTQ files ") +
            "this is 4 (record size), but you could use 1 for e.g. regular text files.",
            false,
            4, "int", cmdLineParser);

//    SwitchArg disablefastqchecks(
//            "d",
//            "disablefastqchecks",
//            "Will (in the future) disable further checks for FASTQ consistency.",
//            false);
//    cmdLineParser.add(disablefastqchecks);

    ValueArg<u_int64_t> startingread(
            "s", "startingread",
            "Defines the first line (multiplied by extractionmultiplier) which should be extracted.",
            false,
            0, "int", cmdLineParser);

    ValueArg<u_int64_t> numberofreads(
            "n", "numberofreads",
            string("Defines the number of reads which should be extracted. The size of each read is defined by ") +
            "extractionmultiplier.",
            false,
            10, "int", cmdLineParser);

    ValueArg<uint> segmentIdentifierArg(
            "S", "segment",
            string("Defines the segment which shall be extracted from the file. This will turn on segment ") +
            "extraction mode and FastqIndEx will ignore startringread and numberofreads. The extractionmultiplier" +
            "parameter will still be used.",
            false,
            0, "uint", cmdLineParser);

    ValueArg<uint> segmentCountArg(
            "N", "segmentcount",
            string("Defines the number of segments for segment extraction mode. The file is virtually divided into") +
            "N segments and you can select the extracted segment using S. Internally, this will be matched to proper"
            "line (record) numbers.",
            false,
            16, "uint", cmdLineParser);

    ValueArg<int> verbosity(
            "v",
            "verbosity",
            string("Sets the verbosity of the application in the range of 0 (default, less) to 3 (debug, max). ") +
            "Invalid values will be ignored and the default of 0 will apply. -D automatically sets the level to 3.",
            false,
            0,
            "int", cmdLineParser);

    SwitchArg debugSwitch(
            "D", "enabledebugging",
            string("Only practicable when you debug the application, e.g. with an IDE. This will tell the extractor ") +
            "component to store various debug information during runtime.",
            cmdLineParser,
            false);

    cmdLineParser.parse(argc, argv);

    ErrorAccumulator::setVerbosity(verbosity.getValue());

    bool dbg = debugSwitch.getValue();
    if (dbg)
        ErrorAccumulator::setVerbosity(3);

    path _indexFile(indexFile.getValue());
    if (indexFile.getValue().empty())
        _indexFile = path(fastqFile.getValue() + ".fqi");

    int _extractionMultiplier = extractionmultiplier.getValue();
    if (_extractionMultiplier <= 0)
        _extractionMultiplier = 0;

    u_int64_t start{0}, count{0};
    ExtractMode extractMode = ExtractMode::lines;
    if (segmentIdentifierArg.isSet()) {
        // Enable segment extraction mode
        start = segmentIdentifierArg.getValue();
        count = segmentCountArg.getValue();
        extractMode = ExtractMode::segment;
    } else {
        start = startingread.getValue() * _extractionMultiplier;
        count = numberofreads.getValue() * _extractionMultiplier;
    }

    return new ExtractorRunner(
            make_shared<PathInputSource>(argumentToPath(fastqFile)),
            _indexFile,
            argumentToPath(outFile),
            forceOverwrite.getValue(),
            extractMode,
            start,
            count,
            _extractionMultiplier,
            debugSwitch.getValue()
    );
}