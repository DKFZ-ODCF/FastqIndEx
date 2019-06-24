/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexModeCLIParser.h"
#include "../process/io/StreamInputSource.h"
#include <tclap/CmdLine.h>

using namespace std;
using namespace TCLAP;

IndexerRunner *IndexModeCLIParser::parse(int argc, const char **argv) {
    // Working with TCLAP is easy but also tricky:
    // - You cannot put the init code in a seperate method (due to scoping issues)
    // - You cannot for some reasons put the variables into the header as there is no constructor for this.
    // So put together everything here.

    CmdLine cmdLineParser("Command description message", '=', "0.0.1", false);

    ValueArg<string> indexFileArg(
            "i",
            "indexfile",
            string("The index file which shall be created or - for stdout or \"\" to append .fqi to the FASTQ filename. ") +
            "Note, that the index will be streamed to stdout, if you provide \"\" or - as the FASTQ file parameter. " +
            "The index file can also reside in an S3 bucket. Enter the filename here like s3:<filename> and set the"
            " bucket with --bucket.",
            false,
            "",
            "string", cmdLineParser);

    ValueArg<string> fastqFileArg(
            "f",
            "fastqfile",
            string("The fastq file which shall be indexed or - for stdin.") +
            "The FASTQ file can also reside in an S3 bucket. Enter the filename here like s3:<filename> and set the"
            " bucket with --bucket.",
            true,
            "-",
            "string", cmdLineParser);

    ValueArg<string> s3BucketArg(
            "",
            "s3bucket",
            "The S3 bucket from where you want to load data.",
            false,
            "", "string", cmdLineParser);

    ValueArg<string> s3EndpointArg(
            "",
            "s3endpoint",
            "The S3 endpoint.",
            false,
            "", "string", cmdLineParser);

    ValueArg<string> s3ConfigFileArg(
            "", "s3config",
            string("In alternative to setting the bucket and the endpoint via command line, you can also specify ") +
            "a configuration file. In this, you can also set proxy settings etc. Create a file with the s3init mode " +
            "if you don't have one. The file format is described on the project website.",
            false,
            "", "string", cmdLineParser);

    ValueArg<int> blockIntervalArg(
            "b",
            "blockinterval",
            string("Defines, that an index entry will be written to the index file for every nth compressed ") +
            "block. Value needs to be in range from [1 .. n].",
            false,
            -1,
            "int", cmdLineParser);

    ValueArg<int> verbosityArg(
            "v",
            "verbosityArg",
            string("Sets the verbosityArg of the application in the range of 0 (default, less) to 3 (debug, max). ") +
            "Invalid values will be ignored and the default of 0 will apply. -D automatically sets the level to 3.",
            false,
            0,
            "int", cmdLineParser);

    SwitchArg forceOverwriteArg(
            "w",
            "forceoverwrite",
            "Allow the indexer to overwrite an existing index file.",
            cmdLineParser);

    SwitchArg dictCompressionSwitch(
            "c", "disabledictionarycompression",
            string("Disable compression of the stored dictionaries. By default, the stored dictionaries will") +
            " be compressed using zlib and this will usually decrease the index size to 1/3rd to 1/4th "
            "compared to an uncompressed index.",
            cmdLineParser, true);

    SwitchArg debugSwitch(
            "D", "enabledebugging",
            string("Only practicable when you debug the application, e.g. with an IDE. This will tell the extractor ") +
            "component to store various debug information during runtime.",
            cmdLineParser,
            false);

    SwitchArg forbidIndexWriteoutSwitch(
            "F", "forbidindexwriteout",
            string("Forbids writing the index file. More interesting for debugging."),
            cmdLineParser,
            false);

    SwitchArg disableFailsafeDistanceSwitch(
            "S", "disablefailsafedistance",
            string("Disables the minimum offset-byte-distance checks for entries in the index file. ") +
            "The failsafe distance is calculated with (block distance) * (16kByte)",
            cmdLineParser,
            false);

    ValueArg<string> storeForDecompressedBlocksArg(
            "L", "storagefordecompressedblocks",
            string("Tell the Indexer to store decompressed blocks to the set location.") +
            " This function should be used with care and is meant for debugging. The target folder must exist.",
            false, "", "string", cmdLineParser);

    ValueArg<string> storeForPartialDecompressedBlocksArg(
            "l", "storageforpartialdecompressedblocks",
            string("Tell the Indexer to store partial information for decompressed blocks to the set location. ") +
            "This function should be used with care and is meant for debugging. The target folder must exist.",
            false, "", "string", cmdLineParser);

    vector<string> allowedMode{"index"};
    ValuesConstraint<string> allowedModesConstraint(allowedMode);
    UnlabeledValueArg<string> mode("mode", "mode is index", true, "", &allowedModesConstraint);
    cmdLineParser.add(mode);

    cmdLineParser.parse(argc, argv);

    shared_ptr<InputSource> fastq = shared_ptr<InputSource>(nullptr);
    path index(indexFileArg.getValue());

    string &fastqFileVal = fastqFileArg.getValue();
    if (fastqFileVal == "-") {      // Streamed mode
        fastq = shared_ptr<InputSource>(new StreamInputSource(&cin));
    } else if (fastqFileVal.size() > 3 && fastqFileVal.substr(0, 3) == "s3:") {     // S3 mode!
    } else {
        fastq = make_shared<PathInputSource>(fastqFileVal);
    }

    if (indexFileArg.getValue().empty()) {
        if (fastq->isStreamSource()) {
            index = "";
        } else {
            index = dynamic_pointer_cast<PathInputSource>(fastq)->absolutePath() + ".fqi";
        }
    }

    int bi = blockIntervalArg.getValue();
    if (bi < -1) bi = -1;

    bool fo = forceOverwriteArg.getValue();

    ErrorAccumulator::setVerbosity(verbosityArg.getValue());
    bool dbg = debugSwitch.getValue();
    if (dbg)
        ErrorAccumulator::setVerbosity(3);

    auto runner = new IndexerRunner(fastq, index, bi, dbg, fo,
                                    forbidIndexWriteoutSwitch.getValue(),
                                    disableFailsafeDistanceSwitch.getValue(),
                                    dictCompressionSwitch.getValue());

    if (storeForDecompressedBlocksArg.isSet())
        runner->enableWriteOutOfDecompressedBlocksAndStatistics(storeForDecompressedBlocksArg.getValue());
    if (storeForPartialDecompressedBlocksArg.isSet()) {
        cerr << "Setting location for partial decompressed block data file to: '"
             << storeForPartialDecompressedBlocksArg.getValue() << "'\n";
        runner->enableWriteOutOfPartialDecompressedBlocks(storeForPartialDecompressedBlocksArg.getValue());
    }
    return runner;
}

