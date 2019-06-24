/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3InputSOURCE_H
#define FASTQINDEX_S3InputSOURCE_H

#include "InputSource.h"
#include "S3Config.h"
#include <aws/core/Aws.h>
#include <fstream>
#include <iostream>

/**
 * InputSource implementation for a path object.
 */
class S3InputSource : public InputSource {
private:

    /**
     * The FASTQ file in the format s3://<bucket>/<file>
     */
    string file;

    path credentials;

    path configuration;

    S3Config s3Config;

    Aws::SDKOptions options;

    Aws::IOStream *source;

public:

    /**
     * Create an instance of this object with a path source. This will be read by fread and so on.
     * @param source
     */
    explicit S3InputSource(const string &file,
                           const path &credentials,
                           const path &configuration,
                           const string &configSection);

    virtual ~S3InputSource();

    bool open() override;

    bool close() override;

    bool exists() override { true; }

    bool isSymlink() { return false; }

    bool isRegularFile() { return true; }

    uint64_t size() override { return -1; }

    string absolutePath() { return "S3"; }

    bool isFileSource() override { return false; };

    bool isStreamSource() override { return true; };

    int read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int seek(int64_t nByte, bool absolute) override;

    int skip(uint64_t nBytes) override;

    uint64_t tell() override;

    bool canRead() override;

    int lastError() override;

    path getPath() { return "S3"; }

};

#include "InputSource.h"

#endif //FASTQINDEX_S3InputSOURCE_H
