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

#include <memory>

using namespace std;
using namespace TCLAP;

IndexerRunner *IndexModeCLIParser::parse(int argc, const char **argv) {
    // Working with TCLAP is easy but also tricky:
    // - You need to put the init code in a seperate method using shared_ptr or pointers (due to scoping issues)
    // - You cannot put the variables into the header as there is no constructor for this.

    auto cmdLineParser = createCommandLineParser();

    // TCLAP displays latest added items first. So read the create..Arg methods in reverse order.
    auto verbosityArg = createVerbosityArg(cmdLineParser.get());

    auto forbidIndexWriteoutSwitch = createForbidIndexWriteoutSwitchArg(cmdLineParser.get());
    auto storeForDecompressedBlocksArg = createStoreForDecompressedBlocksArg(cmdLineParser.get());
    auto storeForPartialDecompressedBlocksArg = createStoreForPartialDecompressedBlocksArg(cmdLineParser.get());
    auto debugSwitch = createDebugSwitchArg(cmdLineParser.get());

    auto disableFailsafeDistanceSwitch = createDisableFailsafeDistanceSwitchArg(cmdLineParser.get());
    auto blockIntervalArg = createBlockIntervalArg(cmdLineParser.get());
    auto byteDistanceArg = createByteDistanceArg(cmdLineParser.get());
    auto[selectIndexMetricArg, constraints] = createSelectIndexEntryStorageStrategyArg(cmdLineParser.get());

    auto forceOverwriteArg = createForceOverwriteSwitchArg(cmdLineParser.get());
    auto dictCompressionSwitch = createDictCompressionSwitchArg(cmdLineParser.get());

    auto s3ConfigFileSectionArg = createS3ConfigFileSectionArg(cmdLineParser.get());
    auto s3CredentialsFileArg = createS3CredentialsFileArg(cmdLineParser.get());
    auto s3ConfigFileArg = createS3ConfigFileArg(cmdLineParser.get());

    auto indexFileArg = createIndexFileArg(cmdLineParser.get());
    auto fastqFileArg = createFastqFileArg(cmdLineParser.get());

    // Keep the mode constraints on the stack, so allowedModeArg won't access invalid memory!
    auto[allowedModeArg, modeConstraints] = createAllowedModeArg("index", cmdLineParser.get());

    cmdLineParser->parse(argc, argv);

    bool forceOverwrite = forceOverwriteArg->getValue();

    S3ServiceOptions s3ServiceOptions(s3ConfigFileArg->getValue(),
                                      s3CredentialsFileArg->getValue(),
                                      s3ConfigFileSectionArg->getValue());
    S3Service::setS3ServiceOptions(s3ServiceOptions);

    auto fastq = processFastqFile(fastqFileArg->getValue(), s3ServiceOptions);
    auto index = processIndexFileSink(indexFileArg->getValue(), forceOverwrite, fastq, s3ServiceOptions);

    shared_ptr<IndexEntryStorageStrategy> storageStrategy;
    if (selectIndexMetricArg->getValue() == "BlockDistance") {
        int blockInterval = blockIntervalArg->getValue();
        if (blockInterval < -1) blockInterval = -1;
        storageStrategy.reset(
                new BlockDistanceStorageStrategy(blockInterval, !disableFailsafeDistanceSwitch->getValue()));
    } else if (selectIndexMetricArg->getValue() == "ByteDistance") {
        storageStrategy.reset(new ByteDistanceStorageStrategy(byteDistanceArg->getValue()));
    }

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

    auto runner = new IndexerRunner(fastq, index, storageStrategy, enableDebugging, forceOverwrite,
                                    forbidIndexWriteoutSwitch->getValue(),
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

tuple<_StringValueArg, shared_ptr<ValuesConstraint<string>>>
IndexModeCLIParser::createSelectIndexEntryStorageStrategyArg(CmdLine *cmdLineParser) const {
    vector<string> allowedMetrics{"ByteDistance", "BlockDistance"};
    auto allowedMetricsConstraint = make_shared<ValuesConstraint<string>>(allowedMetrics);

    auto arg = make_shared<ValueArg<string>>(
            "m", "selectIndexEntryMetric",
            string("Selects the metric which is used to calculate, which compressed block will be referenced with ") +
            "index entry in the resulting FQI file. The default value is \"ByteDistance\".",
            false,
            "ByteDistance", allowedMetricsConstraint.get(), *cmdLineParser);
    return {arg, allowedMetricsConstraint};
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

_StringValueArg IndexModeCLIParser::createByteDistanceArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "B", "byteDistance",
            string("Minimum distance between two referenced compressed blocks in the form of 3k, 4M, 12G or 1TB."),
            false, "-1", cmdLineParser);
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

