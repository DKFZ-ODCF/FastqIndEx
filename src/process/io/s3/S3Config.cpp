/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/IOHelper.h"
#include "S3Config.h"
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <experimental/filesystem>
#include <fstream>
#include <SimpleIni.h>

using namespace std;
using namespace std::experimental::filesystem;

S3Config::S3Config(const S3ServiceOptions &options) :
        credentialsFile(options.credentialsFile),
        configFile(options.configFile) {
    this->configSection = options.configSection;
    if (configSection.empty())
        this->configSection = "default"; // Default section for s3 cli and s3cmd

    readAndValidate();
}

bool S3Config::readAndValidate() {
    bool _valid = figureOutConfigurationFiles();

    if (!_valid) return false;

    _valid = readFiles();

    this->valid = _valid;
    return _valid;
}

bool S3Config::figureOutConfigurationFiles() {
    bool _valid = true;

    this->usedCredentialsFile = path();
    this->usedConfigFile = path();

    path finalCredentialsFile;
    path finalConfigurationFile;

    // try and resolve the configuration file
    finalConfigurationFile = configFile;
    if (finalConfigurationFile.string().empty())
        finalConfigurationFile = getDefaultAWSConfigFile();

    // Try and resolve the credentials file
    finalCredentialsFile = credentialsFile;
    if (finalCredentialsFile.string().empty()) {
        finalCredentialsFile = exists(getDefaultAWSCredentialsFile()) ? getDefaultAWSCredentialsFile()
                                                                      : finalConfigurationFile;
    }

    _valid &= IOHelper::checkFileReadability(finalCredentialsFile, "credentials", this);
    _valid &= IOHelper::checkFileReadability(finalConfigurationFile, "configuration", this);

    S3Config::valid = _valid;
    if (_valid) {
        usedCredentialsFile = finalCredentialsFile;
        usedConfigFile = finalConfigurationFile;
    } else {
        addErrorMessage("Could not validate configuration for S3.");
    }
    return _valid;
}


bool S3Config::readFiles() {

    this->allValues.clear();

    // Config first, then credentials. Specific credentials will overwrite config entries.
    auto configValues = IOHelper::loadIniFile(usedConfigFile, configSection);
    if (usedConfigFile != usedCredentialsFile) {
        auto credentialValues = IOHelper::loadIniFile(usedCredentialsFile, configSection);
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

