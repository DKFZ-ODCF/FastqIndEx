/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "ExtractModeCLIParser.h"
#include "process/io/s3/S3Sink.h"

#include <tclap/CmdLine.h>

using namespace std;
using namespace TCLAP;

ExtractorRunner *ExtractModeCLIParser::parse(int argc, const char **argv) {
    auto cmdLineParser = createCommandLineParser();

    auto verbosityArg = createVerbosityArg(cmdLineParser.get());

    auto debugSwitch = createDebugSwitchArg(cmdLineParser.get());

    auto recordSizeArg = createrecordSizeArg(cmdLineParser.get());
    auto numberOfReadsArg = createNumberOfReadsArg(cmdLineParser.get());
    auto startingReadArg = createStartingReadArg(cmdLineParser.get());

    auto segmentIdentifierArg = createSegmentIdentifierArg(cmdLineParser.get());
    auto segmentCountArg = createSegmentCountArg(cmdLineParser.get());

    auto forceOverwriteArg = createForceOverwriteSwitchArg(cmdLineParser.get());

    auto s3ConfigFileSectionArg = createS3ConfigFileSectionArg(cmdLineParser.get());
    auto s3CredentialsFileArg = createS3CredentialsFileArg(cmdLineParser.get());
    auto s3ConfigFileArg = createS3ConfigFileArg(cmdLineParser.get());

    auto outputFileArg = createOutputFileArg(cmdLineParser.get());
    auto indexFileArg = createIndexFileArg(cmdLineParser.get());
    auto fastqFileArg = createFastqFileArg(cmdLineParser.get());

    // Keep the mode constraints on the stack, so allowedModeArg won't access invalid memory!
    auto[allowedModeArg, modeConstraints] = createAllowedModeArg("extract", cmdLineParser.get());

    cmdLineParser->parse(argc, argv);

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

    uint recordSize = recordSizeArg->getValue();
    if (recordSize <= 0)
        recordSize = 0;

    int64_t start{0}, count{0};
    ExtractMode extractMode = ExtractMode::lines;
    if (segmentIdentifierArg->isSet()) {
        // Enable segment extraction mode
        start = segmentIdentifierArg->getValue();
        count = segmentCountArg->getValue();
        extractMode = ExtractMode::segment;
    } else {
        start = startingReadArg->getValue() * recordSize;
        count = numberOfReadsArg->getValue() * recordSize;
    }

    auto runner = new ExtractorRunner(
            fastq,
            index,
            outputFile,
            forceOverwrite,
            extractMode,
            start,
            count,
            recordSize,
            enableDebugging
    );

    return runner;
}

_StringValueArg ExtractModeCLIParser::createOutputFileArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "o", "outfile",
            string("The uncompressed output FASTQ or textfile which shall be created or - (default) for stdout.") +
            " Also accepts an S3 target.",
            false,
            "-", cmdLineParser);
}

_UIntValueArg ExtractModeCLIParser::createrecordSizeArg(CmdLine *cmdLineParser) const {
    return _makeUIntValueArg(
            "e", "recordSize",
            string("Defines the number of lines in a record. For FASTQ files ") +
            "this is value " + to_string(DEFAULT_RECORD_SIZE) + ", but you could use 1 for e.g. regular text files.",
            false,
            DEFAULT_RECORD_SIZE, cmdLineParser);
}

_UInt64ValueArg ExtractModeCLIParser::createStartingReadArg(CmdLine *cmdLineParser) const {
    return _makeUInt64ValueArg(
            "s", "startingRecord",
            "Defines the first record which shall be extracted.",
            false,
            0, cmdLineParser);
}

_UInt64ValueArg ExtractModeCLIParser::createNumberOfReadsArg(CmdLine *cmdLineParser) const {
    return _makeUInt64ValueArg(
            "n", "numberOfRecords",
            string("Defines the number of records which should be extracted. The size of each read is defined by ") +
            "recordSize.",
            false,
            10, cmdLineParser);
}

_UIntValueArg ExtractModeCLIParser::createSegmentIdentifierArg(CmdLine *cmdLineParser) const {
    return _makeUIntValueArg(
            "S", "segment",
            string("Defines the segment which shall be extracted from the file. This will turn on segment ") +
            "extraction mode and FastqIndEx will ignore startringread and numberofreads. The recordSize" +
            "parameter will still be used.",
            false,
            0, cmdLineParser);
}

_UIntValueArg ExtractModeCLIParser::createSegmentCountArg(CmdLine *cmdLineParser) const {
    return _makeUIntValueArg(
            "N", "segmentCount",
            string("Defines the number of segments for segment extraction mode. The file is virtually divided into") +
            "N segments and you can select the extracted segment using S. Internally, this will be matched to proper"
            "line (record) numbers.",
            false,
            16, cmdLineParser);
}
