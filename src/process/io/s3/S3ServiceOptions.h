/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3SERVICEOPTIONS_H
#define FASTQINDEX_S3SERVICEOPTIONS_H

#include <experimental/filesystem>
#include <string>

using namespace std;
using namespace std::experimental::filesystem;

struct S3ServiceOptions {

    path credentialsFile;

    path configFile;

    string configSection;

    S3ServiceOptions() : S3ServiceOptions(string(""), string(""), "") {}

    S3ServiceOptions(const string &credentialsFile, const string &configFile, const string &configSection);

    S3ServiceOptions(const S3ServiceOptions &opts)
            : S3ServiceOptions(opts.credentialsFile, opts.configFile, opts.configSection) {};
};

#endif //FASTQINDEX_S3SERVICEOPTIONS_H
