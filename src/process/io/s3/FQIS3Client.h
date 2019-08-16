/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_FQIS3CLIENT_H
#define FASTQINDEX_FQIS3CLIENT_H

#include "common/Result.h"
#include "common/StringHelper.h"
#include "process/io/FileSink.h"
#include "process/io/Sink.h"
#include "process/io/s3/S3Config.h"
#include "process/io/s3/S3Service.h"
#include "process/io/s3/S3GetObjectProcessWrapper.h"
#include <cstdio>
#include <fcntl.h>
#include <list>
#include <memory>

#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Aws.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/S3Client.h>

using namespace Aws::Utils;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace std;

typedef Result<bool> FQIS3ClientRequestBooleanResult;

struct S3Object {
    string name;
    u_int64_t size;

    S3Object(const string &name, u_int64_t size) {
        this->name = name;
        this->size = size;
    }
};

/**
 * This could actually be a nice helper class which could be stored as an Object, if needed. But I am working
 */
class FQIS3Client : ErrorAccumulator {

protected:

    S3Service_S service;

    string s3Path;

    S3Config s3Config;

    string bucketName;

    string objectName;

    S3ServiceOptions serviceOptions;

public:

    static shared_ptr<FQIS3Client> from(const string &s3Path, S3Service_S service) {
        return make_shared<FQIS3Client>(s3Path, service);
    }

    FQIS3Client(const string &s3Path, S3Service_S service) {
        this->s3Path = s3Path;
        this->service = service;
        this->s3Config = service->getConfig();
        this->serviceOptions = service->getS3ServiceOptions();
        auto split = StringHelper::splitStr(s3Path, '/'); // s3://bucket/object

        if (split.size() != 4) {
            addErrorMessage("The S3 string '", s3Path, "' must look like s3://<bucket>/<object>");
        } else {
            bucketName = split[2];
            objectName = split[3];
        }
    }

    virtual bool isValid() {
        return service.get() && !bucketName.empty() && !objectName.empty() && s3Config.isValid();
    }

    string getS3Path() {
        return s3Path;
    }

    S3Config getS3Config() {
        return s3Config;
    }

    string getBucketName() {
        return bucketName;
    }

    string getObjectName() {
        return objectName;
    }

    S3ServiceOptions getS3ServiceOptions() {
        return serviceOptions;
    }

    template<typename T>
    bool request(function<T(S3Client &client)> s3Request) {
        bool result;

        auto outcome = s3Request(*service->getClient().get());
        if (!outcome.IsSuccess()) {
            const auto &error = outcome.GetError();
            addErrorMessage("S3 error: ", string(error.GetExceptionName()), ": ", string(error.GetMessage()));
            result = false;
        } else {
            result = true;
        }

        return result;
    }

    virtual Result<list<S3Object>> getObjectList() {
        bool objectExists = false;
        // There is no copy constructor available for result because it has some const fields.
        // If you do not have this as a shared_ptr (or pointer) and try to overwrite it from within the lambda, the
        // compiler will fail and tell you, that the copy constructor was deleted implicitely. The manually added cc
        // does not work as well.
        shared_ptr<Result<list<S3Object>>> result;

        bool requestResult = request<ListObjectsOutcome>([&](S3Client &client) -> ListObjectsOutcome {
            ListObjectsRequest objectRequest;
            objectRequest.SetBucket(bucketName.c_str());
            auto outcome = client.ListObjects(objectRequest);
            std::list<S3Object> entries;
            if (outcome.IsSuccess()) {
                auto objectList = outcome.GetResult().GetContents();
                for (auto const &object : objectList) {
                    entries.emplace_back(S3Object(string(object.GetKey()), static_cast<u_int64_t>(object.GetSize())));
                }
            }
            result = make_shared<Result<list<S3Object>>>(outcome.IsSuccess(), entries);
            return outcome;
        });
        Result<list<S3Object>> _result = *result;
        return _result;
    }

    FQIS3ClientRequestBooleanResult checkObjectExistence() {
        auto result = getObjectList();
        bool found = false;

        if (result.success) {
            for (const auto &entry : result.result) {
                if (entry.name == objectName) {
                    found = true;
                    break;
                }
            }
        }
        return FQIS3ClientRequestBooleanResult(result.success, found);
    }

    /**
     * Returns the object size for this clients object.
     * @return A tuple indicating [success, size]
     */
    virtual Result<uint64_t> getObjectSize() {
        auto result = getObjectList();
        bool found = false;
        uint64_t size = 0;
        if (!result)
            return {false, 0};

        for (const auto &entry : result.result) {
            if (entry.name == objectName) {
                found = true;
                size = entry.size;
                break;
            }
        }
        return {found, size};
    }

    virtual S3GetObjectProcessWrapper_S createS3GetObjectProcessWrapper(const path &fifo, u_int64_t readStart) {
        return make_shared<S3GetObjectProcessWrapper>(getS3ServiceOptions(), fifo, getS3Path(), readStart);
    }

    virtual FQIS3ClientRequestBooleanResult putFile(const string &file) {
        bool couldPut = false;

        bool requestResult = request<PutObjectOutcome>([=](S3Client &client) -> PutObjectOutcome {
            PutObjectRequest objectRequest;
            objectRequest.SetBucket(bucketName.c_str());
            objectRequest.SetKey(objectName.c_str());
            auto inputData =
                    Aws::MakeShared<FStream>("FASTQ index", file.c_str(), std::ios_base::in | std::ios_base::binary);
            objectRequest.SetBody(inputData);
            objectRequest.SetContentMD5(
                    HashingUtils::Base64Encode(HashingUtils::CalculateMD5(*objectRequest.GetBody())));
            return client.PutObject(objectRequest);
        });

        return FQIS3ClientRequestBooleanResult(requestResult, requestResult);
    }

    /**
     * As I had big memory issues when using the same S3 stream for the whole index / extract process, I now try to read
     * things chunk-wise. This might not be the most efficient way, but the memory consumption was around 1.2 times as
     * as the file size. Dealing with larger files (think of 30 or 40GB) is simply not possible.
     * @param position From where to take
     * @param length   How much to take
     * @param buffer   Where to store to. This buffer needs to be large enough to hold the data!
     * @return A tuple with a success indicator and the amount of read Bytes.
     */
    virtual tuple<bool, int64_t> readBlockOfData(int64_t position, int64_t length, Bytef *buffer) {
        int64_t readBytes{0};
        bool success = request<GetObjectOutcome>([&](S3Client &client) -> GetObjectOutcome {
            GetObjectRequest objectRequest;
            objectRequest.SetBucket(bucketName.c_str());
            objectRequest.SetKey(objectName.c_str());
            auto getObjectOutcome = client.GetObject(objectRequest);
            if (getObjectOutcome.IsSuccess()) {
                auto &retrievedFile = getObjectOutcome.GetResult().GetBody();
                retrievedFile.seekg(position, ios_base::beg);
                retrievedFile.readsome(reinterpret_cast<char *>( buffer), length);
            }
            return getObjectOutcome;
        });

        return tuple<bool, int64_t>(success, readBytes);
    }

    vector<string> getErrorMessages() override {
        auto l = ErrorAccumulator::getErrorMessages();
        auto r = s3Config.getErrorMessages();
        return concatenateVectors(l, r);
    }
};

typedef shared_ptr<FQIS3Client> FQIS3Client_S;

#endif //FASTQINDEX_FQIS3CLIENT_H
