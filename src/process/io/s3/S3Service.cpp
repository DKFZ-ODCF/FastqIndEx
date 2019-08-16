/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "S3Service.h"

Aws::SDKOptions S3Service::options;

mutex S3Service::clientInstanceAccessorMutex;

int S3Service::awsServicesCounter = 0;

void S3Service::initializeAWS() {
    lock_guard<mutex> lock(S3Service::clientInstanceAccessorMutex);
    if (!awsServicesCounter) {
        Aws::InitAPI(S3Service::options);
    }
    awsServicesCounter++;
}

void S3Service::shutdownAWS() {
    lock_guard<mutex> lock(S3Service::clientInstanceAccessorMutex);
    awsServicesCounter--;
    if (awsServicesCounter == 0) {
        Aws::ShutdownAPI(S3Service::options);
    }
}
