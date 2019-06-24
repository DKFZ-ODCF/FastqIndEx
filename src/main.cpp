/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include <cstdlib>
#include "main.h"
#include "startup/Starter.h"
#include "process/io/StreamInputSource.h"
#include "process/io/S3InputSource.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <SimpleIni.h>

//void testAWS() {
//    Aws::SDKOptions options;
//
//    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
//    Aws::Environment::SetEnv("AWS_CONFIG_FILE","/data/michael/.aws/config");
//    auto envVar = getenv("AWS_CONFIG_FILE");
//    Aws::InitAPI(options);
//    {
//        // snippet-start:[s3.cpp.get_object.code]
//        // Assign these values before running the program
//        const Aws::String bucket_name = "odcf";
//        const Aws::String object_name = "/AS-133938-LR-18906_R1.fastq.gz_1m.gz";  // For demo, set to a text file
//
//        CSimpleIniA configuration;
//        configuration.SetUnicode(true);
//        auto result = configuration.LoadFile("/data/michael/.aws/config");
//
//        CSimpleIniA::TNamesDepend keys;
//        configuration.GetAllKeys("default", keys);
//        auto value = configuration.GetValue("default", "access_key");
//        // Set up the request
//
//
//        auto cfg = Aws::Client::ClientConfiguration("default");
//        cfg.endpointOverride = "https://s3.dok.cos.dkfz-heidelberg.de";
//        cfg.region = "EU";
//        cfg.verifySSL = false;
//        cfg.connectTimeoutMs = (1000);
//        cfg.requestTimeoutMs = (1000);
////        cfg
////        cfg.proxyHost = "http://www-int2.inet.dkfz-heidelberg.de";
////        cfg.proxyPort = 80;
//
//        auto credentials = Aws::Auth::AWSCredentials();
//        credentials.SetAWSAccessKeyId("aaa");
//        credentials.SetAWSSecretKey("bbb");
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
////            auto is = shared_ptr<InputSource>(new S3InputSource(&retrieved_file));
////            Extractor extractor(
////                    is, "/mnt/sdb/AS-133938-LR-18906_R1.fastq.gz_1m.gz.fqi",
////                    "", false, ExtractMode::lines, 0, 10, 4, true);
////            extractor.extract();
////            retrieved_file.
////            retrieved_file.getline(file_data, 254);
//            std::cout << file_data << std::endl;
//        } else {
//            auto error = get_object_outcome.GetError();
//            std::cout << "ERROR: " << error.GetExceptionName() << ": "
//                      << error.GetMessage() << std::endl;
//        }
//        // snippet-end:[s3.cpp.get_object.code]
//    }
//    Aws::ShutdownAPI(options);
//}

int main(int argc, const char *argv[]) {
//    testAWS();
    Starter *starter = Starter::getInstance();
    shared_ptr<Runner> runner = starter->createRunner(argc, argv);

    int exitCode = 0;
    if (!runner->checkPremises()) {
        cerr << "There were errors preventing FastqIndEx to start:\n";
        for (auto const &message : runner->getErrorMessages()) {
            cerr << "\t" << message << "\n";
        }
        exitCode = 1;
    } else {
        exitCode = runner->run();
    }
    delete starter;
    return exitCode;
}