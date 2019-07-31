/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexModeCLIParser.h"
#include "process/io/StreamSource.h"
#include "process/io/s3/S3Source.h"
#include "process/io/s3/S3Sink.h"
#include "ModeCLIParser.h"
#include <tclap/CmdLine.h>

using namespace std;
using namespace TCLAP;

const string  IndexModeCLIParser::descriptionForIndexModeIndexFileArg =
        string("The index file which shall be created or - for stdout or \"\" to append .fqi to the FASTQ filename. ") +
        "Note, that the index will be streamed to stdout, if you provide \"\" or - as the FASTQ file parameter. " +
        "The index file can also reside in an S3 bucket. Enter the filename here like s3:<filename> and set the"
        " bucket with --bucket." +
        " Note, that this software will create the index in the tmp directory before uploading it to S3!";

const string  IndexModeCLIParser::descriptForFastqFileArg =
        string("The fastq file which shall be indexed or - for stdin.") +
        "The FASTQ file can also reside in an S3 bucket. Enter the filename here like s3:<filename> and set the"
        " bucket with --bucket.";

const string  IndexModeCLIParser::s3ConfigFileSectionArgDescription =
        string("In alternative to setting the bucket and the endpoint via command line, you can also specify ") +
        "a configuration file. In this, you can also set proxy settings etc. Create a file with the s3init mode " +
        "if you don't have one. The file format is described on the project website.";

IndexerRunner *IndexModeCLIParser::parse(int argc, const char **argv) {
    // Working with TCLAP is easy but also tricky:
    // - You need to put the init code in a seperate method using shared_ptr or pointers (due to scoping issues)
    // - You cannot put the variables into the header as there is no constructor for this.

    CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);

    auto fastqFileArg = createFastqFileArg(&cmdLineParser);

    auto indexFileArg = createIndexFileArg(&cmdLineParser);

    auto s3ConfigFileArg = createS3ConfigFileArg(&cmdLineParser);

    auto s3CredentialsFileArg = createS3CredentialsFileArg(&cmdLineParser);

    auto s3ConfigFileSectionArg = createS3ConfigFileSectionArg(&cmdLineParser);

    auto blockIntervalArg = createBlockIntervalArg(&cmdLineParser);

    auto verbosityArg = createVerbosityArg(&cmdLineParser);

    auto forceOverwriteArg = createForceOverwriteSwitchArg(&cmdLineParser);

    auto dictCompressionSwitch = createDictCompressionSwitchArg(&cmdLineParser);

    auto debugSwitch = createDebugSwitchArg(&cmdLineParser);

    auto forbidIndexWriteoutSwitch = createForbidIndexWriteoutSwitchArg(&cmdLineParser);

    auto disableFailsafeDistanceSwitch = createDisableFailsafeDistanceSwitchArg(&cmdLineParser);

    auto storeForDecompressedBlocksArg = createStoreForDecompressedBlocksArg(&cmdLineParser);

    auto storeForPartialDecompressedBlocksArg = createStoreForPartialDecompressedBlocksArg(&cmdLineParser);

    vector<string> allowedMode{"index"};
    ValuesConstraint<string> allowedModesConstraint(allowedMode);
    UnlabeledValueArg<string> mode("mode", "mode is index", true, "", &allowedModesConstraint);
    cmdLineParser.add(mode);

    cmdLineParser.parse(argc, argv);

    bool forceOverwrite = forceOverwriteArg->getValue();

    S3ServiceOptions s3ServiceOptions(s3ConfigFileArg->getValue(),
                                      s3CredentialsFileArg->getValue(),
                                      s3ConfigFileSectionArg->getValue());
    S3Service::setS3ServiceOptions(s3ServiceOptions);

    auto fastq = processFastqFile(fastqFileArg->getValue(), s3ServiceOptions);

    auto index = processIndexFileSink(indexFileArg->getValue(), forceOverwrite, fastq, s3ServiceOptions);

    int blockInterval = blockIntervalArg->getValue();
    if (blockInterval < -1) blockInterval = -1;

    ErrorAccumulator::setVerbosity(verbosityArg->getValue());
    bool enableDebugging = debugSwitch->getValue();
    if (enableDebugging)
        ErrorAccumulator::setVerbosity(3);

    ErrorAccumulator::always("FASTQ file: '", fastq->toString(), "'");
    ErrorAccumulator::always("Index file: '", index->toString(), "'");
    ErrorAccumulator::always("Index compression is turned ", dictCompressionSwitch->getValue() ? "on" : "off");

    if (forceOverwrite)
        ErrorAccumulator::always("FQI file can be overwritten");
    if (enableDebugging) {
        ErrorAccumulator::always("Debugging is on");
    }
    if (forbidIndexWriteoutSwitch->getValue())
        ErrorAccumulator::always("FQI file must not be written");
    if (disableFailsafeDistanceSwitch->getValue())
        ErrorAccumulator::always("Failsafe distance is turned off");

    auto runner = new IndexerRunner(fastq, index, blockInterval, enableDebugging, forceOverwrite,
                                    forbidIndexWriteoutSwitch->getValue(),
                                    disableFailsafeDistanceSwitch->getValue(),
                                    dictCompressionSwitch->getValue());

    if (storeForDecompressedBlocksArg->isSet()) {
        runner->enableWriteOutOfDecompressedBlocksAndStatistics(storeForDecompressedBlocksArg->getValue());
        ErrorAccumulator::always("Storage location for decompressed blocks: '",
                                 storeForDecompressedBlocksArg->getValue(), "'");
    }

    if (storeForPartialDecompressedBlocksArg->isSet()) {
        ErrorAccumulator::always("Storage location for partial decompressed block data file to: '",
                                 storeForPartialDecompressedBlocksArg->getValue(), "'");
        runner->enableWriteOutOfPartialDecompressedBlocks(storeForPartialDecompressedBlocksArg->getValue());
    }
    return runner;
}

_IntValueArg IndexModeCLIParser::createBlockIntervalArg(CmdLine *cmdLineParser) const {
    return _makeIntValueArg(
            "b",
            "blockInterval",
            string("Defines, that an index entry will be written to the index file for every nth compressed ") +
            "block. Value needs to be in range from [1 .. n].",
            false,
            -1, cmdLineParser);
}

_SwitchArg IndexModeCLIParser::createDictCompressionSwitchArg(CmdLine *cmdLineParser) const {
    return _makeSwitchArg(
            "c", "disableDictionaryCompression",
            string("Disable compression of the stored dictionaries. By default, the stored dictionaries will") +
            " be compressed using zlib and this will usually decrease the index size to 1/3rd to 1/4th "
            "compared to an uncompressed index.",
            cmdLineParser, true);
}

_SwitchArg IndexModeCLIParser::createForbidIndexWriteoutSwitchArg(CmdLine *cmdLineParser) const {
    return _makeSwitchArg(
            "F", "forbidIndexWrite",
            string("Forbids writing the index file. More interesting for debugging."),
            cmdLineParser);
}

_SwitchArg IndexModeCLIParser::createDisableFailsafeDistanceSwitchArg(CmdLine *cmdLineParser) const {
    return _makeSwitchArg(
            "S", "disableFailsafeDistance",
            string("Disables the minimum offset-byte-distance checks for entries in the index file. ") +
            "The failsafe distance is calculated with (block distance) * (16kByte)",
            cmdLineParser);
}

_StringValueArg IndexModeCLIParser::createStoreForPartialDecompressedBlocksArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "l", "storageForPartialDecompressedBlocks",
            string("Tell the Indexer to store partial information for decompressed blocks to the set location. ") +
            "This function should be used with care and is meant for debugging. The target folder must exist.",
            false, "", cmdLineParser);
}

_StringValueArg IndexModeCLIParser::createStoreForDecompressedBlocksArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "L", "storageForDecompressedBlocks",
            string("Tell the Indexer to store decompressed blocks to the set location.") +
            " This function should be used with care and is meant for debugging. The target folder must exist.",
            false, "", cmdLineParser);
}

