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

    S3Service_S service;

    bool _isOpen{false};

    bool sizeRequested{false};

    int64_t _size{0};

    int64_t position{0};

    shared_ptr<FQIS3Client> fqiS3Client;

    path fifo;

    S3GetObjectProcessWrapper_S s3GetObjectWrapper;

    FStream *s3FStream{nullptr};

    ifstream stream;

    shared_ptr<StreamSource> streamSource;

    mutex signalDisablingMutex;

public:

    const path &getFifo() const;

    const ifstream &getStream() {
        return stream;
    }

    static shared_ptr<S3Source> from(const string &s3Path, S3Service_S service) {
        return from(FQIS3Client::from(s3Path, service));
    }

    static shared_ptr<S3Source> from(FQIS3Client_S client) {
        return make_shared<S3Source>(client);
    }

    /**
     * Create an instance of this object with a path source. This will be read by fread and so on.
     * @param source
     */
    explicit S3Source(FQIS3Client_S client);

    ~S3Source() override;

    void setReadStart(int64_t startBytes) override;

    bool fulfillsPremises() override;

    bool open() override;

    bool openWithReadLock() override;

    bool close() override;

    bool isOpen() override;

    bool hasLock() override;

    bool unlock() override;

    bool eof() override;

    bool isGood() override;

    bool isFile() override { return true; };

    bool isStream() override { return true; };

    bool isSymlink() override { return false; }

    bool exists() override {
        auto res = fqiS3Client->checkObjectExistence();
        return res && res.result;
    }

    int64_t size() override {
        if (!sizeRequested) {
            _size = fqiS3Client->getObjectSize().result;
            sizeRequested = true;
        }
        return _size;
    }

    bool empty() override;

    bool canRead() override;

    bool canWrite() override;

    int64_t getTotalReadBytes() override;

    string absolutePath() { return "S3"; }

    int64_t read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int64_t seek(int64_t nByte, bool absolute) override;

    int64_t skip(int64_t nBytes) override;

    int64_t rewind(int64_t nByte) override;

    int64_t tell() override;

    int lastError() override;

    vector<string> getErrorMessages() override {
        auto l = ErrorAccumulator::getErrorMessages();
        auto r = fqiS3Client->getErrorMessages();
        if (!streamSource.get()) {
            return concatenateVectors(l, r);
        } else {
            auto s = streamSource->getErrorMessages();
            return concatenateVectors(l, r, s);
        }
    }

    string toString() override;
};

#include "process/io/Source.h"

#endif //FASTQINDEX_S3Source_H
