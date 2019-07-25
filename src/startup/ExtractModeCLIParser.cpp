/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "ExtractModeCLIParser.h"
#include "process/io/s3/S3Sink.h"
#include "process/io/s3/S3Source.h"

#include <tclap/CmdLine.h>

using namespace std;
using namespace TCLAP;

ExtractorRunner *ExtractModeCLIParser::parse(int argc, const char **argv) {
    CmdLine cmdLineParser("Extract reads from a FASTQ file (or lines from a text file).", '=', "0.0.1", false);

    auto outputFileArg = createOutputFileArg(&cmdLineParser);

    auto fastqFileArg = createFastqFileArg(&cmdLineParser);

    auto indexFileArg = createIndexFileArg(&cmdLineParser);

    auto s3ConfigFileArg = createS3ConfigFileArg(&cmdLineParser);

    auto s3CredentialsFileArg = createS3CredentialsFileArg(&cmdLineParser);

    auto s3ConfigFileSectionArg = createS3ConfigFileSectionArg(&cmdLineParser);

    auto forceOverwriteArg = createForceOverwriteSwitchArg(&cmdLineParser);

    auto extractionMultiplierArg = createExtractionMultiplierArg(&cmdLineParser);

    auto startingReadArg = createStartingReadArg(&cmdLineParser);

    auto numberOfReadsArg = createNumberOfReadsArg(&cmdLineParser);

    auto segmentIdentifierArg = createSegmentIdentifierArg(&cmdLineParser);

    auto segmentCountArg = createSegmentCountArg(&cmdLineParser);

    auto verbosityArg = createVerbosityArg(&cmdLineParser);

    auto debugSwitch = createDebugSwitchArg(&cmdLineParser);

    vector<string> allowedMode{"extract"};
    ValuesConstraint<string> allowedModesConstraint(allowedMode);
    UnlabeledValueArg<string> mode("mode", "mode is extract", true, "", &allowedModesConstraint);
    cmdLineParser.add(mode);

    cmdLineParser.parse(argc, argv);

    S3ServiceOptions s3ServiceOptions(s3ConfigFileArg->getValue(),
                                      s3CredentialsFileArg->getValue(),
                                      s3ConfigFileSectionArg->getValue());
    S3Service::setS3ServiceOptions(s3ServiceOptions);

    auto fastq = processFastqFile(fastqFileArg->getValue(), s3ServiceOptions);

    auto index = processIndexFileSource(indexFileArg->getValue(), fastq, s3ServiceOptions);

    bool forceOverwrite = forceOverwriteArg->getValue();

    auto outputFile = processFileSink(outputFileArg->getValue(), forceOverwrite, s3ServiceOptions);

    ErrorAccumulator::setVerbosity(verbosityArg->getValue());
    bool enableDebugging = debugSwitch->getValue();
    if (enableDebugging)
        ErrorAccumulator::setVerbosity(3);

    int _extractionMultiplier = extractionMultiplierArg->getValue();
    if (_extractionMultiplier <= 0)
        _extractionMultiplier = 0;

    u_int64_t start{0}, count{0};
    ExtractMode extractMode = ExtractMode::lines;
    if (segmentIdentifierArg->isSet()) {
        // Enable segment extraction mode
        start = segmentIdentifierArg->getValue();
        count = segmentCountArg->getValue();
        extractMode = ExtractMode::segment;
    } else {
        start = startingReadArg->getValue() * _extractionMultiplier;
        count = numberOfReadsArg->getValue() * _extractionMultiplier;
    }

    auto runner = new ExtractorRunner(
            fastq,
            index,
            outputFile,
            forceOverwrite,
            extractMode,
            start,
            count,
            _extractionMultiplier,
            enableDebugging
    );

    return runner;
}

_StringValueArg ExtractModeCLIParser::createOutputFileArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "o", "outfile",
            "The uncompressed output FASTQ or textfile which shall be created or - (default) for stdout.",
            false,
            "-", cmdLineParser);
}

_IntValueArg ExtractModeCLIParser::createExtractionMultiplierArg(CmdLine *cmdLineParser) const {
    return _makeIntValueArg(
            "e", "extractionmultiplier",
            string("Defines a multiplier by which the startingline parameter will be multiplied. For FASTQ files ") +
            "this is 4 (record size), but you could use 1 for e.g. regular text files.",
            false,
            4, cmdLineParser);
}

_UInt64ValueArg ExtractModeCLIParser::createStartingReadArg(CmdLine *cmdLineParser) const {
    return _makeUInt64ValueArg(
            "s", "startingread",
            "Defines the first line (multiplied by extractionmultiplier) which should be extracted.",
            false,
            0, cmdLineParser);
}

_UInt64ValueArg ExtractModeCLIParser::createNumberOfReadsArg(CmdLine *cmdLineParser) const {
    return _makeUInt64ValueArg(
            "n", "numberofreads",
            string("Defines the number of reads which should be extracted. The size of each read is defined by ") +
            "extractionmultiplier.",
            false,
            10, cmdLineParser);
}

_UIntValueArg ExtractModeCLIParser::createSegmentIdentifierArg(CmdLine *cmdLineParser) const {
    return _makeUIntValueArg(
            "S", "segment",
            string("Defines the segment which shall be extracted from the file. This will turn on segment ") +
            "extraction mode and FastqIndEx will ignore startringread and numberofreads. The extractionmultiplier" +
            "parameter will still be used.",
            false,
            0, cmdLineParser);
}

_UIntValueArg ExtractModeCLIParser::createSegmentCountArg(CmdLine *cmdLineParser) const {
    return _makeUIntValueArg(
            "N", "segmentcount",
            string("Defines the number of segments for segment extraction mode. The file is virtually divided into") +
            "N segments and you can select the extracted segment using S. Internally, this will be matched to proper"
            "line (record) numbers.",
            false,
            16, cmdLineParser);
}
