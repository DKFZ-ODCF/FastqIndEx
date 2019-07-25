/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3SINK_H
#define FASTQINDEX_S3SINK_H

#include "common/StringHelper.h"
#include "process/io/PathSink.h"
#include "process/io/Sink.h"
#include "S3Config.h"
#include "FQIS3Client.h"
#include <cstdio>
#include <fcntl.h>
#include <memory>

#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Aws.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/S3Client.h>

using namespace Aws::Utils;
using namespace Aws::S3;
using namespace Aws::S3::Model;

class S3Sink : public Sink {
private:

    bool _isOpen{false};

    bool objectAlreadyExists{false};

    PathSink *tempFile{nullptr};

    FQIS3Client fqiS3Client;

public:

    static shared_ptr<S3Sink> from(const string &file, bool forceOverwrite, const S3ServiceOptions &s3ServiceOptions) {
        return make_shared<S3Sink>(file, forceOverwrite, s3ServiceOptions);
    }

    S3Sink(const string &s3Path, bool forceOverwrite, const S3ServiceOptions &s3ServiceOptions) :
            fqiS3Client(s3Path, s3ServiceOptions),
            Sink(forceOverwrite) {
    }

    virtual ~S3Sink() {
        close();
    }

    bool openWithWriteLock() override {
        return open();
    }

    bool checkPremises() override {
        return true;
    }


    bool open() override {
        if (isOpen())
            return true;

        if (!fqiS3Client.isValid()) {
            return false;
        }

        // Assign these values before running the program
        bool ok = true;

        auto result = fqiS3Client.checkObjectExistence();
        if (!result.requestWasSuccessful)
            return false;

        objectAlreadyExists = result.result;

        if (objectAlreadyExists && !this->forceOverwrite) {
            addErrorMessage("The file '", this->fqiS3Client.getS3Path(),
                            "' already exists. ",
                            "Use -w to force the application to overwrite the file.");
            return false;
        }

        auto[success, tempFilePath] = IOHelper::createTempFile("FastqIndEx_S3TemporaryIndex");
        if (!success) {
            addErrorMessage("Could not create a temporary file for the S3 sink");
            return false;
        }

        this->tempFile = new PathSink(tempFilePath, true);
        if (!tempFile->openWithWriteLock()) {
            delete tempFile;
            tempFile = nullptr;
            return false;
        }

        _isOpen = ok;
        return ok;
    }

    bool close() override {
        if (!isOpen())
            return true;

        bool ok = true;
        if (ok) {
            auto result = fqiS3Client.putFile(tempFile->toString());
            ok = result.requestWasSuccessful && result.result;
            if (result.result)
                ErrorAccumulator::always("Uploaded FQI file to bucket s3://", fqiS3Client.getBucketName());
        }


        if (tempFile) {
            ok = tempFile->close();
            if (tempFile->exists())
                remove(tempFile->getPath());
            delete tempFile;
            tempFile = nullptr;
        }
        _isOpen = false;
        return ok;
    }

    bool isOpen() override {
        return tempFile && _isOpen;
    }

    bool eof() override {
        return tempFile && tempFile->eof();
    }

    bool isGood() override {

        return tempFile && tempFile->isGood();
    }

    bool isFile() override {
        return true;
    }

    bool isStream() override {
        return true;
    }

    bool isSymlink() override {
        return false;
    }

    bool exists() override {
        return tempFile && tempFile->exists();
    }

    u_int64_t size() override {
        return tempFile ? tempFile->size() : 0;
    }

    bool empty() override {
        return tempFile ? tempFile->empty() : true;
    }

    bool canRead() override {
        return tempFile && tempFile->canRead();
    }

    bool canWrite() override {
        //Also check for existence...
        return tempFile && tempFile->canWrite();
    }

    int seek(int64_t nByte, bool absolute) override {
        return tempFile ? tempFile->seek(nByte, absolute) : 0;
    }

    int skip(uint64_t nByte) override {
        return tempFile ? tempFile->skip(nByte) : 0;
    }

    string toString() override {
        return fqiS3Client.getS3Path();
    }

    uint64_t tell() override {
        return tempFile ? tempFile->tell() : 0;
    }

    int lastError() override {
        return 0;
    }

    void write(const char *message) override {
        if (tempFile)tempFile->write(message);
    }

    void write(const char *message, int len) override {
        if (tempFile) tempFile->write(message, len);
    }

    void write(const string &message) override {
        if (tempFile) tempFile->write(message);
    }

    void flush() override {
        if (tempFile) tempFile->flush();
    }

    vector<string> getErrorMessages() override {
        auto l = ErrorAccumulator::getErrorMessages();
        auto r = fqiS3Client.getErrorMessages();
        if (!tempFile) {
            return mergeToNewVector(l, r);
        } else {
            auto s = tempFile->getErrorMessages();
            return mergeToNewVector(l, r, s);
        }
    }

};

#endif //FASTQINDEX_S3SINK_H
