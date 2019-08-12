/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_FQIS3CLIENT_H
#define FASTQINDEX_FQIS3CLIENT_H

#include "common/StringHelper.h"
#include "process/io/s3/S3Service.h"
#include "process/io/PathSink.h"
#include "process/io/Sink.h"
#include "process/io/s3/S3Config.h"
#include <cstdio>
#include <fcntl.h>
#include <memory>

#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Aws.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/S3Client.h>
#include <list>

using namespace Aws::Utils;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace std;

struct FQIS3ClientRequestBooleanResult {
    bool requestWasSuccessful;
    bool result;

    FQIS3ClientRequestBooleanResult(bool requestWasSuccessful, bool result) :
            requestWasSuccessful(requestWasSuccessful), result(result) {}
};

struct ObjectListEntry {
    string name;
    int64_t size;

    ObjectListEntry(const string &name, int64_t size) {
        this->name = name;
        this->size = size;
    }
};

/**
 * This could actually be a nice helper class which could be stored as an Object, if needed. But I am working
 */
class FQIS3Client : ErrorAccumulator {

protected:

    string s3Path;

    S3Config s3Config;

    string bucket_name;

    string object_name;

    S3ServiceOptions serviceOptions;

public:

    FQIS3Client(const string &s3Path,
                const S3ServiceOptions &s3ServiceOptions) {
        this->s3Path = s3Path;
        this->s3Config = S3Service::getInstance()->getConfig();
        this->serviceOptions = s3ServiceOptions;
        auto split = StringHelper::splitStr(s3Path, '/'); // s3://bucket/object

        if (split.size() != 4) {
            addErrorMessage("The S3 string '", s3Path, "' must look like s3://<bucket>/<object>");
        } else {
            bucket_name = split[2];
            object_name = split[3];
        }
    }

    bool isValid() {
        return S3Service::getInstance().get() && !bucket_name.empty() && !object_name.empty() && s3Config.isValid();
    }

    string getS3Path() {
        return s3Path;
    }

    S3Config getS3Config() {
        return s3Config;
    }

    string getBucketName() {
        return bucket_name;
    }

    string getObjectName() {
        return object_name;
    }

    S3ServiceOptions getS3ServiceOptions() {
        return serviceOptions;
    }


    template<typename T>
    bool performS3Request(function<T(S3Client &client)> s3Request) {
        bool result;

        auto outcome = s3Request(*S3Service::getInstance()->getClient().get());
        if (!outcome.IsSuccess()) {
            const auto &error = outcome.GetError();
            addErrorMessage("S3 error: ", string(error.GetExceptionName()), ": ", string(error.GetMessage()));
            result = false;
        } else {
            result = true;
        }

        return result;
    }

    tuple<bool, std::list<ObjectListEntry>> getObjectList() {
        bool objectExists = false;
        tuple<bool, std::list<ObjectListEntry>> result;

        bool requestResult = performS3Request<ListObjectsOutcome>([&](S3Client &client) -> ListObjectsOutcome {
            ListObjectsRequest object_request;
            object_request.SetBucket(bucket_name.c_str());
            auto outcome = client.ListObjects(object_request);
            std::list<ObjectListEntry> entries;
            if (outcome.IsSuccess()) {
                auto object_list = outcome.GetResult().GetContents();
//                cerr << "Checking S3 bucket for files:\n";
                for (auto const &s3_object : object_list) {
//                    std::cerr << "* " << s3_object.GetKey() << std::endl;
                    entries.emplace_back(ObjectListEntry(string(s3_object.GetKey()), s3_object.GetSize()));
                }
            }
            result = tuple<bool, std::list<ObjectListEntry>>(outcome.IsSuccess(), entries);
            return outcome;
        });
        return result;
    }

    FQIS3ClientRequestBooleanResult checkObjectExistence() {
        auto list = getObjectList();
        bool found = false;
        if (std::get<0>(list)) {
            for (const auto &entry : std::get<1>(list)) {
                if (entry.name == object_name) {
                    found = true;
                    break;
                }
            }
        }
        return FQIS3ClientRequestBooleanResult(std::get<0>(list), found);
    }

    /**
     * Returns the object size for this clients object.
     * @return A tuple indicating [success, size]
     */
    tuple<bool, int64_t> getObjectSize() {
        auto[success, list] = getObjectList();
        bool found = false;
        int64_t size = 0;
        if (!success)
            return {false, 0};

        for (const auto &entry : list) {
            if (entry.name == object_name) {
                found = true;
                size = entry.size;
                break;
            }
        }
        return {found, size};
    }

    FQIS3ClientRequestBooleanResult putFile(const string &file) {
        bool couldPut = false;

        bool requestResult = performS3Request<PutObjectOutcome>([=](S3Client &client) -> PutObjectOutcome {
            PutObjectRequest object_request;
            object_request.SetBucket(bucket_name.c_str());
            object_request.SetKey(object_name.c_str());
            auto input_data =
                    Aws::MakeShared<FStream>("FASTQ index", file.c_str(), std::ios_base::in | std::ios_base::binary);
            object_request.SetBody(input_data);
            object_request.SetContentMD5(
                    HashingUtils::Base64Encode(HashingUtils::CalculateMD5(*object_request.GetBody())));
            return client.PutObject(object_request);
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
    tuple<bool, int64_t> readBlockOfData(int64_t position, int64_t length, Bytef *buffer) {
        int64_t readBytes{0};
        bool success = performS3Request<GetObjectOutcome>([&](S3Client &client) -> GetObjectOutcome {
            GetObjectRequest object_request;
            object_request.SetBucket(bucket_name.c_str());
            object_request.SetKey(object_name.c_str());
            auto get_object_outcome = client.GetObject(object_request);
            if (get_object_outcome.IsSuccess()) {
                auto &retrieved_file = get_object_outcome.GetResult().GetBody();
                retrieved_file.seekg(position, ios_base::beg);
                retrieved_file.readsome(reinterpret_cast<char *>( buffer), length);
            }
            return get_object_outcome;
        });

        return tuple<bool, int64_t>(success, readBytes);
    }

    vector<string> getErrorMessages() override {
        auto l = ErrorAccumulator::getErrorMessages();
        auto r = s3Config.getErrorMessages();
        return mergeToNewVector(l, r);
    }
};

#endif //FASTQINDEX_FQIS3CLIENT_H
