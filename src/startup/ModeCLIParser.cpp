/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/FileSink.h"
#include "process/io/FileSource.h"
#include "process/io/s3/S3Sink.h"
#include "process/io/s3/S3Source.h"
#include "process/io/Source.h"
#include "process/io/ConsoleSink.h"
#include "process/io/StreamSource.h"
#include "ModeCLIParser.h"
#include <tclap/CmdLine.h>

#include <memory>

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
    return _makeStringValueArg(
            "i", "indexFile",
            string("The index file which shall be created or - for stdout or \"\" to append .fqi to the FASTQ filename. ") +
            "Note, that the index will be streamed to stdout, if you provide \"\" or - as the FASTQ file parameter. " +
            "The index file can also reside in an S3 bucket. Enter the filename here like s3:<filename> and set the"
            " bucket with --bucket." +
            " Note, that this software will create the index in the tmp directory before uploading it to S3!",
            false, "",
            cmdLineParser);
}

_StringValueArg ModeCLIParser::createFastqFileArg(CmdLine *cmdLineParser) const {
    return _makeStringValueArg(
            "f", "sourceFile",
            string("The FASTQ file which shall be indexed or - for stdin.") +
            "The FASTQ file can also reside in an S3 bucket. Enter the filename here like s3:<filename> and set the"
            " bucket with --bucket.",
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
            string("In alternative to setting the bucket and the endpoint via command line, you can also specify ") +
            "a configuration file. In this, you can also set proxy settings etc. Create a file with the s3init mode " +
            "if you don't have one. The file format is described on the project website.",
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

tuple<_UnlabedeledStringValueArg, _ValuesConstraint>
ModeCLIParser::createAllowedModeArg(const string &mode, CmdLine *cmdLineParser) const {
    vector<string> allowedMode{mode};
    auto allowedModesConstraint = make_shared<ValuesConstraint<string>>(allowedMode);
    auto arg = std::make_shared<UnlabeledValueArg<string>>(mode, "mode is " + mode, true, "",
                                                           allowedModesConstraint.get());
    cmdLineParser->add(arg.get());
    return {arg, allowedModesConstraint};
}

shared_ptr<Source> ModeCLIParser::processSourceFileSource(const string &sourceFileArg,
                                                          S3Service_S s3Service) {
    if (sourceFileArg == "-") {
        return StreamSource::from(&cin);
    } else if (isS3Path(sourceFileArg)) {
        return S3Source::from(FQIS3Client::from(sourceFileArg, s3Service));
    }
    return FileSource::from(sourceFileArg);
}

shared_ptr<Source> ModeCLIParser::processIndexFileSource(const string &indexFile,
                                                         const shared_ptr<Source> &fastqSource,
                                                         S3Service_S s3Service) {
    string _indexFile = resolveIndexFileName(indexFile, fastqSource);

    return processIndexFileSource(_indexFile, s3Service);
}

shared_ptr<Source> ModeCLIParser::processIndexFileSource(const string &indexFile,
                                                         S3Service_S s3Service) {
    if (isS3Path(indexFile)) {
        return S3Source::from(FQIS3Client::from(indexFile, s3Service));
    } else {
        return FileSource::from(indexFile);
    } // Index might still be empty but this will be checked later!
}

shared_ptr<Sink> ModeCLIParser::processIndexFileSink(const string &_indexFile,
                                                     bool forceOverwrite,
                                                     const shared_ptr<Source> &fastqSource,
                                                     S3Service_S s3Service) {
    string indexFile = resolveIndexFileName(_indexFile, fastqSource);

    if (isS3Path(indexFile)) {
        return S3Sink::from(FQIS3Client::from(indexFile, s3Service), forceOverwrite);
    } else {
        return FileSink::from(indexFile, forceOverwrite);
    }
}

shared_ptr<Sink> ModeCLIParser::processFileSink(const string &file,
                                                bool forceOverwrite,
                                                S3Service_S s3Service) {
    if (file == "-") {
        return ConsoleSink::create();
    } else if (isS3Path(file)) {
        return S3Sink::from(FQIS3Client::from(file, s3Service), forceOverwrite);
    } else {
        return FileSink::from(file, forceOverwrite);
    }
}