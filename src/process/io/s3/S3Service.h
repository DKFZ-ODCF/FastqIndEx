/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3SERVICE_H
#define FASTQINDEX_S3SERVICE_H

#include "process/io/s3/S3ServiceOptions.h"
#include "process/io/s3/S3Config.h"

#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Aws.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/S3Client.h>

#include <experimental/filesystem>
#include <string>
#include <mutex>

using namespace Aws::Auth;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace Aws::Utils;

using namespace std;
using namespace std::experimental::filesystem;

/**
 * Thread safe helper service for S3
 */
class S3Service {

private:

    static mutex clientInstanceAccessorMutex;

    static shared_ptr<S3Service> instance;

    static S3ServiceOptions serviceOptions;

    shared_ptr<S3Client> client;

    S3Config config;

    Aws::SDKOptions options;

    static shared_ptr<S3Service> initialize() {
        if (!instance.get())
            instance = shared_ptr<S3Service>(new S3Service());
        return instance;
    }

    S3Service() {
        Aws::InitAPI(options);
        config = S3Config(S3Service::serviceOptions);
        auto credentials = Aws::Auth::AWSCredentials();
        auto cfg = Aws::Client::ClientConfiguration();
        config.fillAWSCredentials(credentials);
        config.fillAWSClientConfiguration(cfg);
        client = shared_ptr<S3Client>(new S3Client(credentials, cfg));
    };
public:

    /**
     * Use this to set the s3 service options BEFORE you call getInstance (which will initialize the service)
     * This will not reinitialize the SDK or the instance! Use close, if you want to renew settings.
     */
    static void setS3ServiceOptions(const S3ServiceOptions &s3Opts) {
        lock_guard<mutex> lock(S3Service::clientInstanceAccessorMutex);
        S3Service::serviceOptions = s3Opts;
    }

    static shared_ptr<S3Service> getInstance() {
        lock_guard<mutex> lock(S3Service::clientInstanceAccessorMutex);
        if (!instance.get()) {
            initialize();
        }
        return instance;
    }

    static void closeIfOpened() {
        lock_guard<mutex> lock(S3Service::clientInstanceAccessorMutex);

        // If the instance is open, close it by resetting the shared_ptr.
        instance.reset();
    }

    virtual ~S3Service() {
        Aws::ShutdownAPI(options);
    }

    shared_ptr<S3Client> getClient() {
        return client;
    }

    S3Config &getConfig() {
        return config;
    }
};

#endif //FASTQINDEX_S3SERVICE_H
