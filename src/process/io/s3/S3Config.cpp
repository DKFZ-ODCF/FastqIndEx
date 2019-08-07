/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/IOHelper.h"
#include "S3Config.h"
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <experimental/filesystem>
#include <fstream>
#include <SimpleIni.h>

using namespace std;
using namespace std::experimental::filesystem;

S3Config::S3Config(const S3ServiceOptions &options) :
        requestedCredentialsFile(options.credentialsFile),
        requestedConfigFile(options.configFile) {
    this->selectedConfigSection = options.configSection;
    if (selectedConfigSection.empty())
        this->selectedConfigSection = "default"; // Default section for s3 cli and s3cmd

    readAndValidate();
}

bool S3Config::readAndValidate() {
    bool _isValid = figureOutConfigurationFiles();

    if (!_isValid) return false;

    _isValid = readFiles();

    this->_isValid = _isValid;
    return _isValid;
}

bool S3Config::figureOutConfigurationFiles() {
    bool _isValid = true;

    this->usedCredentialsFile = path();
    this->usedConfigFile = path();

    path finalCredentialsFile;
    path finalConfigurationFile;

    // try and resolve the configuration file
    finalConfigurationFile = requestedConfigFile;
    if (finalConfigurationFile.string().empty())
        finalConfigurationFile = getDefaultAWSConfigFile();

    // Try and resolve the credentials file
    finalCredentialsFile = requestedCredentialsFile;
    if (finalCredentialsFile.string().empty()) {
        finalCredentialsFile = exists(getDefaultAWSCredentialsFile()) ? getDefaultAWSCredentialsFile()
                                                                      : finalConfigurationFile;
    }

    _isValid &= IOHelper::checkFileReadability(finalCredentialsFile, "credentials", this);
    _isValid &= IOHelper::checkFileReadability(finalConfigurationFile, "configuration", this);

    S3Config::_isValid = _isValid;
    if (_isValid) {
        usedCredentialsFile = finalCredentialsFile;
        usedConfigFile = finalConfigurationFile;
    } else {
        addErrorMessage("Could not validate configuration for S3.");
    }
    return _isValid;
}


bool S3Config::readFiles() {

    this->allValues.clear();

    // Config first, then credentials. Specific credentials will overwrite config entries.
    auto configValues = IOHelper::loadIniFile(usedConfigFile, selectedConfigSection);
    if (usedConfigFile != usedCredentialsFile) {
        auto credentialValues = IOHelper::loadIniFile(usedCredentialsFile, selectedConfigSection);
        for (auto const&[key, val] : *credentialValues) {
            (*configValues)[key] = val;
        }
    }
    this->allValues = map<string, string>(*configValues);

    return true;
}

void S3Config::fillAWSCredentials(Aws::Auth::AWSCredentials &credentials) {
    credentials.SetAWSSecretKey(getAWSSecretKey().c_str());
    credentials.SetAWSAccessKeyId(getAWSAccessKeyId().c_str());
}

void S3Config::fillAWSClientConfiguration(Aws::Client::ClientConfiguration &configuration) {
    configuration.proxyScheme = getProxyScheme();
    configuration.proxyHost = getProxyHost();
    configuration.proxyPort = getProxyPort();
    configuration.caFile = getCaFile();
    configuration.endpointOverride = getEndpointOverride();
    configuration.region = getRegion();
    configuration.userAgent = getUserAgent();
    configuration.followRedirects = getFollowRedirects();
    configuration.connectTimeoutMs = getConnectTimeoutMs();
    configuration.verifySSL = getVerifySSL();
}

string S3Config::getAWSAccessKeyId() { return allValues["access_key"]; }

string S3Config::getAWSSecretKey() { return allValues["secret_key"]; }

