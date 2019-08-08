/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "S3Service.h"

mutex S3Service::clientInstanceAccessorMutex;

S3ServiceOptions S3Service::serviceOptions;

shared_ptr<S3Service> S3Service::instance;

S3ServiceOptions::S3ServiceOptions(
        const string &credentialsFile,
        const string &configFile,
        const string &configSection) {
    this->credentialsFile = path(credentialsFile);
    this->configFile = path(configFile);
    this->configSection = configSection;
}