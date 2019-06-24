/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "../../common/StringHelper.h"
#include "S3InputSource.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>

S3InputSource::S3InputSource(const string &file,
                             const path &credentials,
                             const path &configuration,
                             const string &configSection) {
    this->file = file;
    this->configuration = configuration;
    this->credentials = credentials;
    this->s3Config = S3Config(credentials, configuration, configSection);
}

S3InputSource::~S3InputSource() {
    close();
}

bool S3InputSource::open() {
    if(!s3Config.isValid()) {
        return false;
    }

//    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        auto split = StringHelper::splitStr(file, '/'); // s3://bucket/object

        if(split.size() != 4) {
            stringstream sstream;
            sstream << "The provided s3 string '" << file << "' is malformated. It must be like s3://<bucket>/<object>";
            addErrorMessage(sstream.str());
            close();
            return false;
        }

        // Assign these values before running the program
        const Aws::String bucket_name = split[2];
        const Aws::String object_name = split[3];

        auto credentials = Aws::Auth::AWSCredentials();
        auto cfg = Aws::Client::ClientConfiguration();

        s3Config.fillAWSCredentials(credentials);
        s3Config.fillAWSClientConfiguration(cfg);

//        // Set up the request
//        cfg.endpointOverride = "https://s3.dok.cos.dkfz-heidelberg.de";
//        cfg.region = "EU";
//        cfg.verifySSL = false;
//        cfg.connectTimeoutMs = (1000);
//        cfg.requestTimeoutMs = (1000);
////        cfg
////        cfg.proxyHost = "http://www-int2.inet.dkfz-heidelberg.de";
////        cfg.proxyPort = 80;
//
//        credentials.SetAWSAccessKeyId("hoDcCQ47o4BuR1GT2f6e");
//        credentials.SetAWSSecretKey("MZ8OQryctiUSh6F5QZaPFnfXuh3t59cCdYYYqsXG");
//
//        Aws::S3::S3Client s3_client(credentials, cfg);
//
//        auto outcome = s3_client.ListBuckets();
//
//
//        Aws::S3::Model::GetObjectRequest object_request;
//
//        object_request.SetBucket(bucket_name);
//        object_request.SetKey(object_name);
//
//
//        // Get the object
//        auto get_object_outcome = s3_client.GetObject(object_request);
//        if (get_object_outcome.IsSuccess()) {
//            // Get an Aws::IOStream reference to the retrieved file
//            auto &retrieved_file = get_object_outcome.GetResultWithOwnership().GetBody();
//
//            // Output the first line of the retrieved text file
//            std::cout << "Beginning of file contents:\n";
//            char file_data[255] = {0};
////            retrieved_file.
//            auto is = shared_ptr<InputSource>(new S3InputSource(&retrieved_file));
//            std::cout << file_data << std::endl;
//        } else {
//            auto error = get_object_outcome.GetError();
//            std::cout << "ERROR: " << error.GetExceptionName() << ": "
//                      << error.GetMessage() << std::endl;
//        }
    }

    return true;
}

bool S3InputSource::close() {
    Aws::ShutdownAPI(options);
    return true;
}

int S3InputSource::read(Bytef *targetBuffer, int numberOfBytes) {
    source->read(reinterpret_cast<char *>(targetBuffer), numberOfBytes);
    int amountRead = source->gcount();
    return (int) amountRead;
}

int S3InputSource::readChar() {
    Byte result = 0;
    int res = this->read(&result, 1);
    return res < 0 ? res : (int) result;
}

int S3InputSource::seek(int64_t nByte, bool absolute) {
    if (lastError()) {
        // Seek / Read can run over file borders and it might be necessary to just reopen it. We do this here.
        close();
        if (!open()) return 0;
    }

    if (absolute)
        source->seekg(nByte, std::ifstream::beg);
    else
        source->seekg(nByte, std::ifstream::cur);
    return (!source->fail() && !source->bad()) ? 1 : 0;
}

int S3InputSource::skip(uint64_t nBytes) {
    return seek(nBytes, false);
}

uint64_t S3InputSource::tell() {
    return source->tellg();
}

bool S3InputSource::canRead() {
    return tell() < size();
}

int S3InputSource::lastError() {
    return source->fail() || source->bad();
}
