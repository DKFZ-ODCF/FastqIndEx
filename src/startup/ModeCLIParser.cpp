/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/PathSink.h"
#include "process/io/PathSource.h"
#include "process/io/s3/S3Sink.h"
#include "process/io/s3/S3Source.h"
#include "process/io/Source.h"
#include "process/io/ConsoleSink.h"
#include "process/io/StreamSource.h"
#include "runners/IndexerRunner.h"
#include "runners/ExtractorRunner.h"
#include "IndexModeCLIParser.h"
#include "ExtractModeCLIParser.h"
#include "ModeCLIParser.h"
#include "Starter.h"
#include <tclap/CmdLine.h>
#include <cstring>

path ModeCLIParser::argumentToPath(ValueArg<string> &cliArg) {
    path _path;
    if (cliArg.getValue() == "-")
        _path = path("-");
    else
        _path = path(cliArg.getValue());
    return _path;
}

_IntValueArg ModeCLIParser::createVerbosityArg(CmdLine *cmdLineParser) const {
    return _makeIntValueArg(
            "v",
            "verbosity",
            string("Sets the verbosityArg of the application in the range of 0 (default, less) to 3 (debug, max). ") +
            "Invalid values will be ignored and the default of 0 will apply. -D automatically sets the level to 3.",
            false,
            0, cmdLineParser);
}

_SwitchArg ModeCLIParser::createDebugSwitchArg(CmdLine *cmdLineParser) const {
    return _makeSwitchArg(
            "D", "enableDebugging",
            string("Only practicable when you debug the application, e.g. with an IDE. This will tell the extractor ") +
            "component to store various debug information during runtime.",
            cmdLineParser);
}

_StringValueArg ModeCLIParser::createIndexFileArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg("i", "indexFile",
                               IndexModeCLIParser::descriptionForIndexModeIndexFileArg, false, "",
                               cmdLineParser);
}

_StringValueArg ModeCLIParser::createFastqFileArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "f", "fastqFile",
            IndexModeCLIParser::descriptForFastqFileArg,
            true,
            "-",
            cmdLineParser);
}

_StringValueArg ModeCLIParser::createS3CredentialsFileArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "", "s3CredentialsFile",
            string("In alternative to setting the bucket and the endpoint via command line, you can also specify ") +
            "a configuration file. In this, you can also set proxy settings etc. Create a file with the s3init mode " +
            "if you don't have one. The file format is described on the project website.",
            false,
            "", cmdLineParser);
}

_StringValueArg ModeCLIParser::createS3ConfigFileArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "", "s3ConfigFile",
            string("In alternative to setting the bucket and the endpoint via command line, you can also specify ") +
            "a configuration file. In this, you can also set proxy settings etc. Create a file with the s3init mode " +
            "if you don't have one. The file format is described on the project website.",
            false,
            "", cmdLineParser);
}

_StringValueArg ModeCLIParser::createS3ConfigFileSectionArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "", "s3ConfigSection",
            IndexModeCLIParser::s3ConfigFileSectionArgDescription,
            false,
            "default",
            cmdLineParser);
}

_SwitchArg ModeCLIParser::createForceOverwriteSwitchArg(CmdLine *cmdLineParser) const {
    return _makeSwitchArg(
            "w",
            "forceOverwrite",
            "Allow the indexer to overwrite an existing index file.",
            cmdLineParser);
}

shared_ptr<Source> ModeCLIParser::processFastqFile(const string &fastqFileVal,
                                                   const S3ServiceOptions &s3ServiceOptions) {

    if (fastqFileVal == "-") {      // Streamed mode
        return StreamSource::from(&cin);
    } else if (isS3Path(fastqFileVal)) {     // S3 mode!
        return S3Source::from(fastqFileVal, s3ServiceOptions);
    }
    return PathSource::from(fastqFileVal);
}

shared_ptr<Source> ModeCLIParser::processIndexFileSource(const string &indexFile,
                                                         const shared_ptr<Source> &fastqSource,
                                                         const S3ServiceOptions &s3ServiceOptions) {
    string _indexFile = resolveIndexFileName(indexFile, fastqSource);

    return processIndexFileSource(_indexFile, s3ServiceOptions);
}

shared_ptr<Source> ModeCLIParser::processIndexFileSource(const string &indexFile,
                                                         const S3ServiceOptions &s3ServiceOptions) {
    if (isS3Path(indexFile)) {
        return S3Source::from(indexFile, s3ServiceOptions);
    } else {
        return PathSource::from(indexFile);
    } // Index might still be empty but this will be checked later!
}

shared_ptr<Sink> ModeCLIParser::processIndexFileSink(const string &_indexFile,
                                                     bool forceOverwrite,
                                                     const shared_ptr<Source> &fastqSource,
                                                     const S3ServiceOptions &s3ServiceOptions) {
    string indexFile = resolveIndexFileName(_indexFile, fastqSource);

    if (isS3Path(indexFile)) {
        return S3Sink::from(indexFile, forceOverwrite, s3ServiceOptions);
    } else {
        return PathSink::from(indexFile, forceOverwrite);
    }
}

shared_ptr<Sink> ModeCLIParser::processFileSink(const string &file,
                                                bool forceOverwrite,
                                                const S3ServiceOptions &s3ServiceOptions) {
    string fileValue = file;
    if (fileValue == "-") {
        return ConsoleSink::create();
    } else if (isS3Path(fileValue)) {
        return S3Sink::from(fileValue, forceOverwrite, s3ServiceOptions);
    } else {
        return PathSink::from(fileValue, forceOverwrite);
    }
}