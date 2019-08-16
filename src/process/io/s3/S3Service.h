/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3SERVICE_H
#define FASTQINDEX_S3SERVICE_H

#include "process/io/s3/S3Config.h"
#include "process/io/s3/S3ServiceOptions.h"

#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Aws.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/S3Client.h>

#include <experimental/filesystem>
#include <memory>
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

    static Aws::SDKOptions options;

    static mutex clientInstanceAccessorMutex;

    static int awsServicesCounter;

    S3ServiceOptions serviceOptions;

    shared_ptr<S3Client> client;

    S3Config config;


    /**
     * It must be guarantee, that Aws::InitAPI is only called once.
     */
    static void initializeAWS();

    static void shutdownAWS();

public:

    static int getAWSInstanceCount() {
        return awsServicesCounter;
    }

    static shared_ptr<S3Service> getDefault() {
        return from(S3ServiceOptions());
    }

    static shared_ptr<S3Service> from(const S3ServiceOptions &s3ServiceOptions) {
        return make_shared<S3Service>(s3ServiceOptions);
    }

    S3Service(const S3ServiceOptions &serviceOptions) {
        initializeAWS();
        this->serviceOptions = serviceOptions;
        config = S3Config(serviceOptions);
        auto credentials = Aws::Auth::AWSCredentials();
        auto cfg = Aws::Client::ClientConfiguration();
        config.fillAWSCredentials(credentials);
        config.fillAWSClientConfiguration(cfg);
        client = std::make_shared<S3Client>(credentials, cfg);
    }

    virtual ~S3Service() {
        shutdownAWS();
    }

    S3ServiceOptions &getS3ServiceOptions() {
        return serviceOptions;
    }

    shared_ptr<S3Client> getClient() {
        return client;
    }

    S3Config &getConfig() {
        return config;
    }
};

typedef std::shared_ptr<S3Service> S3Service_S;

#endif //FASTQINDEX_S3SERVICE_H
