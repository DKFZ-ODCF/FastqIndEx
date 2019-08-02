/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_MODECLIPARSER_H
#define FASTQINDEX_MODECLIPARSER_H


#include "runners/Runner.h"
#include "runners/IndexerRunner.h"
#include "process/io/Sink.h"
#include "startup/ModeCLIParser.h"
#include <exception>
#include <iostream>
#include <string_view>
#include <tclap/CmdLine.h>

using namespace TCLAP;

typedef shared_ptr<SwitchArg> _SwitchArg;
typedef shared_ptr<ValueArg<string>> _StringValueArg;
typedef shared_ptr<ValueArg<int>> _IntValueArg;
typedef shared_ptr<ValueArg<uint>> _UIntValueArg;
typedef shared_ptr<ValueArg<u_int64_t>> _UInt64ValueArg;

class ModeCLIParser : public ErrorAccumulator {

protected:

    static _StringValueArg
    _makeStringValueArg(string _short, string _long, string _desc, bool req, string _default, CmdLine *parser) {
        return make_shared<ValueArg<string>>(_short, _long, _desc, req, _default, "string", *parser);
    }

    static _IntValueArg
    _makeIntValueArg(string _short, string _long, string _desc, bool req, int _default, CmdLine *parser) {
        return make_shared<ValueArg<int>>(_short, _long, _desc, req, _default, "int", *parser);
    }

    static _UIntValueArg
    _makeUIntValueArg(string _short, string _long, string _desc, bool req, int _default, CmdLine *parser) {
        return make_shared<ValueArg<uint>>(_short, _long, _desc, req, _default, "uint", *parser);
    }

    static _UInt64ValueArg
    _makeUInt64ValueArg(string _short, string _long, string _desc, bool req, int _default, CmdLine *parser) {
        return make_shared<ValueArg<u_int64_t>>(_short, _long, _desc, req, _default, "u_int64_t", *parser);
    }

    static _SwitchArg
    _makeSwitchArg(string _short, string _long, string _desc, CmdLine *parser, bool _default = false) {
        return make_shared<SwitchArg>(_short, _long, _desc, *parser, _default);
    }

public:

    /**
     * Needs to be overriden by specialized mode parsers and is used to parse the application arguments.
     * Note, that subclasses may use derived classes of Runner as a return type (C++ allows this). However, e.g. CLion
     * will not recognize parse as used!
     */
    virtual Runner *parse(int argc, const char **argv) = 0;

    shared_ptr<CmdLine> createCommandLineParser() {
        return make_shared<CmdLine>(
                "FastqInDex - A tool to index compressed FASTQ (or text files) and to allow random data extraction from them.",
                '=', "0.10b", false);
    }

    _IntValueArg createVerbosityArg(CmdLine *cmdLineParser) const;

    _SwitchArg createDebugSwitchArg(CmdLine *cmdLineParser) const;

    _StringValueArg createIndexFileArg(CmdLine *cmdLineParser) const;

    _StringValueArg createFastqFileArg(CmdLine *cmdLineParser) const;

    _StringValueArg createS3CredentialsFileArg(CmdLine *cmdLineParser) const;

    _StringValueArg createS3ConfigFileArg(CmdLine *cmdLineParser) const;

    _StringValueArg createS3ConfigFileSectionArg(CmdLine *cmdLineParser) const;

    _SwitchArg createForceOverwriteSwitchArg(CmdLine *cmdLineParser) const;

    tuple<shared_ptr<UnlabeledValueArg<string>>, shared_ptr<ValuesConstraint<string>>>
    createAllowedModeArg(const string &mode, CmdLine *cmdLineParser) const;
    
    static bool isS3Path(string str) {
        string_view _str = str;
        string prefix = "s3://";
        return _str.size() > prefix.size() && _str.substr(0, prefix.size()) == prefix;
    }

    static string resolveIndexFileName(const string &file, const shared_ptr<Source> &fastq) {
        // S3 and Path based! Stream is not a valid input here, as it has no name. In that case, the original value will
        // be returned.
        if (file.empty() && fastq->isFile()) {
            return fastq->toString() + ".fqi";
        }
        return file;
    }

    /**
     * Processes several FASTQ related parameters and create a FASTQ PathSource from the input.
     */
    static shared_ptr<Source> processFastqFile(const string &fastqFileArg, const S3ServiceOptions &s3ServiceOptions);

    static shared_ptr<Source> processIndexFileSource(const string &indexFile,
                                                     const shared_ptr<Source> &fastqSource,
                                                     const S3ServiceOptions &s3ServiceOptions);

    static shared_ptr<Source> processIndexFileSource(const string &indexFile, const S3ServiceOptions &s3ServiceOptions);

    static shared_ptr<Sink> processIndexFileSink(const string &_indexFile,
                                                 bool forceOverwrite,
                                                 const shared_ptr<Source> &fastqSource,
                                                 const S3ServiceOptions &s3ServiceOptions);

    static shared_ptr<Sink> processFileSink(const string &file,
                                            bool forceOverwrite,
                                            const S3ServiceOptions &s3ServiceOptions);
};


#endif //FASTQINDEX_MODECLIPARSER_H
