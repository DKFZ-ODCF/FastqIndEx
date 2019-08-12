/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3Source_H
#define FASTQINDEX_S3Source_H

#include "S3Config.h"
#include "S3GetObjectProcessWrapper.h"
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
 * This class enables you to get data from an S3 bucket. The way, how we retrieve data, might seem odd. Unfortunately,
 * we experienced quite a number of issues with the Amazon AWS SDK and eventually decided to do everything like this.
 * - The SDK does not allow to work directly with the data stream. Therefore, we decided to download the data to
 *   a named pipe in the tmp folder and access this with a StreamSource
 * - Due to this, it is not possible to seek within an S3 file stream! Fortunately, the StreamSource has some built-in
 *   buffer to deal with this.
 * - It is not possible to abort a running request! What we did first was download with GetObjectAsync. However this
 *   lead to errors, when we closed the pipe or tried to close the stream. Worst case: The application crashed, best
 *   case: It didn't crash but always showed an error (SIGPIPE / SIGABRT). Personally, my trust in application is not
 *   very high, if it shows me such types of errors.
 *   Eventually we moved the download to a separate process. It works but has no real error handling by now. But: It
 *   swallows any bad error codes and messages like a broken pipe (which is not nice but intentional in this case).
 * Unfortunately, we are not able to really know, how much data we need for extraction and we need the possibility to
 * abort a Get request at any time.
 *
 * With the current implementation, things work so far but we still need to think about error handling and reporting.
 */
class S3Source : public Source {

private:

    bool _isOpen{false};

    bool sizeRequested{false};

    int64_t _size{0};

    int64_t position{0};

    FQIS3Client fqiS3Client;

    path fifo;

    shared_ptr<S3GetObjectProcessWrapper> s3GetObjectWrapper;

    FStream *s3FStream{nullptr};

    ifstream stream;

    shared_ptr<StreamSource> streamSource;

    mutex signalDisablingMutex;

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

    ~S3Source() override;

    static void get_object_async_finished(const Aws::S3::S3Client *client,
                                          const Aws::S3::Model::GetObjectRequest &request,
                                          const Aws::S3::Model::GetObjectOutcome &outcome,
                                          const std::shared_ptr<const Aws::Client::AsyncCallerContext> &context) {
        if (outcome.IsSuccess()) {
            std::cerr << "get_object_async_finished: " << context->GetUUID() << std::endl;
        } else {
            const auto& error = outcome.GetError();
            std::cerr << "ERROR: " << error.GetExceptionName() << ": "
                      << error.GetMessage() << std::endl;
        }

        // The example works with a thread notification. However, we are dealing with a fifo which is blocking until it
        // fully read. So we do not need a notification. I'll just leave it in here for further reference.
        //    upload_variable.notify_one();
    }

    bool openS3() {
        if (s3FStream)
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

    bool fulfillsPremises() override;

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

    int64_t getTotalReadBytes() override;

    bool isSymlink() override { return false; }

    bool isRegularFile() { return true; }

    int64_t size() override {
        if (!sizeRequested) {
            _size = std::get<1>(fqiS3Client.getObjectSize());
            sizeRequested = true;
        }
        return _size;
    }

    string absolutePath() { return "S3"; }

    bool isFile() override { return true; };

    bool isStream() override { return true; };

    int64_t read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int64_t seek(int64_t nByte, bool absolute) override;

    int64_t skip(int64_t nBytes) override;

    int64_t tell() override;

    bool canRead() override;

    int lastError() override;

    int64_t rewind(int64_t nByte) override;

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

    void setReadStart(int64_t startBytes) override;


};

#include "process/io/Source.h"

#endif //FASTQINDEX_S3Source_H
