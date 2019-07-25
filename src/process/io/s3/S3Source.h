/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3Source_H
#define FASTQINDEX_S3Source_H

#include "S3Config.h"
#include "process/io/Source.h"
#include "process/io/StreamSource.h"
#include "FQIS3Client.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <fstream>
#include <iostream>
#include <semaphore.h>

/**
 * Source implementation for a path object.
 */
class S3Source : public Source {

private:

    bool _isOpen{false};

    bool sizeRequested{false};

    u_int64_t _size{0};

    u_int64_t position{0};

    FQIS3Client fqiS3Client;

    path fifo;

    /** The S3Source needs its own options as it works with asynchronous features. **/

    shared_ptr<Aws::S3::S3Client> client;

    Aws::SDKOptions options;

    FStream* s3FStream{nullptr};

    ifstream stream;

    shared_ptr<StreamSource> streamSource;

    sem_t asyncGetSemaphore;

public:

    const path &getFifo() const;

    const ifstream &getStream() {
        return stream;
    }

    static shared_ptr<S3Source> from(const string &file, const S3ServiceOptions &s3ServiceOptions) {
        return make_shared<S3Source>(file, s3ServiceOptions);
    }

    /**
     * Create an instance of this object with a path source. This will be read by fread and so on.
     * @param source
     */
    explicit S3Source(const string &s3Path, const S3ServiceOptions &s3ServiceOptions);

    virtual ~S3Source();

    static void get_object_async_finished(const Aws::S3::S3Client *client,
                                          const Aws::S3::Model::GetObjectRequest &request,
                                          const Aws::S3::Model::GetObjectOutcome &outcome,
                                          const std::shared_ptr<const Aws::Client::AsyncCallerContext> &context) {
        if (outcome.IsSuccess()) {
            std::cerr << "get_object_async_finished: " << context->GetUUID() << std::endl;
        } else {
            auto error = outcome.GetError();
            std::cerr << "ERROR: " << error.GetExceptionName() << ": "
                      << error.GetMessage() << std::endl;
        }

        // The example works with a thread notification. However, we are dealing with a fifo which is blocking until it
        // fully read. So we do not need a notification. I'll just leave it in here for further reference.
        //    upload_variable.notify_one();
    }

    bool openS3() {
        if(s3FStream)
            return true;

        auto s3 = S3Service::getInstance();

        // Create and start asynchronous request
        Aws::S3::Model::GetObjectRequest object_request;
        object_request.SetBucket(fqiS3Client.getBucketName().c_str());
        object_request.SetKey(fqiS3Client.getObjectName().c_str());

        if (readStart != 0) {
            auto rangeString = string("bytes=") + to_string(readStart) + "-" + to_string(size());
            object_request.SetRange(rangeString.c_str());
        }
        string object = fqiS3Client.getObjectName();
        object_request.SetResponseStreamFactory([&]() {
            always("Writing ", fqiS3Client.getObjectName(), " with S3 to ", this->fifo);
            auto stream = Aws::New<FStream>(fqiS3Client.getObjectName().c_str(), this->fifo.c_str(),
                                            std::ios_base::out | std::ios_base::binary);
            this->s3FStream = stream;
            return stream;
        });

        auto context = Aws::MakeShared<Aws::Client::AsyncCallerContext>("GetObjectAllocationTag");
        context->SetUUID(fqiS3Client.getObjectName().c_str());
        auto client = s3->getClient();
        s3->getClient()->GetObjectAsync(object_request, get_object_async_finished, context);
        // Will wait until stream was produced.

        this->stream.open(this->fifo, ios_base::in | ios_base::binary);
        this->streamSource = StreamSource::from(&this->stream);
        return true;
    }

    bool checkPremises() override;

    bool isOpen() override;

    bool eof() override;

    bool isGood() override;

    bool empty() override;

    bool canWrite() override;

    string toString() override;

    bool openWithReadLock() override;

    bool open() override;

    bool close() override;

    bool exists() override {
        return true;
    }

    bool hasLock() override;

    bool unlock() override;

    const u_int64_t getTotalReadBytes() override;

    bool isSymlink() { return false; }

    bool isRegularFile() { return true; }

    uint64_t size() override {
        if (!sizeRequested) {
            _size = std::get<1>(fqiS3Client.getObjectSize());
            sizeRequested = true;
        }
        return _size;
    }

    string absolutePath() { return "S3"; }

    bool isFile() override { return true; };

    bool isStream() override { return true; };

    int read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int seek(int64_t nByte, bool absolute) override;

    int skip(uint64_t nBytes) override;

    uint64_t tell() override;

    bool canRead() override;

    int lastError() override;

    int rewind(uint64_t nByte) override;

    vector<string> getErrorMessages() override {
        auto l = ErrorAccumulator::getErrorMessages();
        auto r = fqiS3Client.getErrorMessages();
        if (!streamSource.get()) {
            return mergeToNewVector(l, r);
        } else {
            auto s = streamSource->getErrorMessages();
            return mergeToNewVector(l, r, s);
        }
    }

    void setReadStart(u_int64_t startBytes) override;


};

#include "process/io/Source.h"

#endif //FASTQINDEX_S3Source_H
