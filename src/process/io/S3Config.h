/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3CONFIG_H
#define FASTQINDEX_S3CONFIG_H

#include "../../common/ErrorAccumulator.h"
#include "../../common/IOHelper.h"
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/ClientConfiguration.h>
#include <experimental/filesystem>

using namespace Aws;
using namespace Aws::Https;
using namespace Aws::Utils;
using namespace Aws::Utils::Threading;
using namespace std;
using namespace std::experimental::filesystem;

class S3Config : public ErrorAccumulator {

private:

    Aws::Client::ClientConfiguration defaultConfiguration;

    path requestedCredentialsFile;

    path requestedConfigFile;

    path usedCredentialsFile;

    path usedConfigFile;

    /**
     * May be empty upon construction but is then set to default.
     *
     * Identifies a section within an ini file like
     *
     * [default]
     * key = value
     *
     * [section 31]
     * key = value
     */
    string selectedConfigSection;

    /**
     * Map of all loaded values
     */
    map<string, string> allValues;

    /**
     * If an error occurred during construction, this is stored here. Error messages are retrievable via
     * getErrorMessages()
     */
    bool _isValid{false};

    /**
     * Called to resolve the credential and configuration file.
     *
     * Will set credentialsFile and configFile.
     *
     * @return true, if this went fine.
     */
    bool figureOutConfigurationFiles();

protected:

    /**
     * This could be static but might be overridden by e.g. Test classes
     * @return
     */
    virtual path getDefaultAWSCredentialsFile() {
        return path(IOHelper::getUserHomeDirectory().string() + string("/.aws/credentials"));
    }


    /**
     * This could be static but might be overridden by e.g. Test classes
     * @return
     */
    virtual path getDefaultAWSConfigFile() {
        return path(IOHelper::getUserHomeDirectory().string() + string("/.aws/config"));
    }

public:

    S3Config() {};

    /**
     * When constructing an S3Config object, you need at least a configuration file. We here try to adapt the aws cli
     * method which, if not specified otherwise, will take an existing ~/.aws/config file and ~/.aws/credentials
     * likewise. Credentials can be stored in the config file and do not necessariliy need to be placed in the
     * credentials file.
     *
     * The credential file will mask entries in the config file.
     *
     * @param credentialsFile        A (valid) path to an existing file or an empty path object.
     * @param configFile             A (valid) path to an existing file or an empty path object.
     * @param selectedConfigSection  The section identifier for the configuration files "default" by default
     */
    S3Config(const path &credentialsFile, const path &configFile, const string &selectedConfigSection);

    /**
     * Resolve the configuration files, read them OR finally return false, if something went wrong. You can use
     * getErrorMessages to retrieve a vector with all reported error messages.
     * @return
     */
    bool readAndValidate();

    /**
     * Tells you, if this object is usable or not.
     */
    bool isValid() { return _isValid; };

    /**
     * The selected configuration section in the configuration file ("default" by default)
     */
    string getSelectedConfigSection() { return selectedConfigSection; };

    /**
     * Get the (resolved) credentials file, this might be the config file, if no credentials file was and there is
     * no defeault credentials file available.
     */
    path getCredentialsFile() { return usedCredentialsFile; }

    /**
     * Get the (resolved) config file
     */
    path getConfigFile() { return usedConfigFile; }

    virtual bool readFiles();

    void fillAWSCredentials(Aws::Auth::AWSCredentials &credentials);

    void fillAWSClientConfiguration(Aws::Client::ClientConfiguration &configuration);

    bool hasOptionSet(string key) {
        auto iterator = allValues.find(key);
        bool has = allValues.end() != iterator;
        allValues.erase(iterator); // According to the cppreference example
    }

    string getStringSafe(string key, string _default = "") {
        if (!hasOptionSet(key))
            return _default;
        return allValues[key];
    }

    int getIntSafe(string key, int _default = 0) {
        string value = getStringSafe(key, to_string(_default));
        int result = _default;
        try {
            result = stoi(allValues["socket_timeout"]);
        } catch (invalid_argument) {
            addErrorMessage("String '", value,
                            "' is missing or cannot be converted to an integer.");
        } catch (out_of_range) {
            addErrorMessage("String '", value,
                            "' is too large for an integer conversion.");
        }
        return result;
    }

    int getBoolSafe(string key, bool _default = 0) {
        string value = getStringSafe(key, to_string(_default));
        string str = value;
        bool result = _default;
        try {
            transform(str.begin(), str.end(), str.begin(), ::tolower);
            if (str == "true")
                result = true;
            else if (str == "false")
                result = false;
            else
                throw invalid_argument("");

        } catch (invalid_argument) {
            addErrorMessage("String '", value,
                            "' is missing or cannot be converted to bool. Must be either [T/t]rue or [F/f]alse.");
        }
        return result;
    }

    string getAWSAccessKeyId();

    string getAWSSecretKey();


    int getSocketTimeout() { return getIntSafe("socket_timeout", defaultConfiguration.connectTimeoutMs); }

    Scheme getProxyScheme() { return getStringSafe("", defaultConfiguration.proxyScheme); }

    Scheme getScheme() { return getStringSafe("", defaultConfiguration.scheme); }

    TransferLibType getHttpLibOverride() { return getStringSafe("", defaultConfiguration.httpLibOverride); }

    String getCaFile() { return getStringSafe("", defaultConfiguration.caFile); }

    String getCaPath() { return getStringSafe("", defaultConfiguration.caPath); }

    String getEndpointOverride() { return getStringSafe("", defaultConfiguration.endpointOverride); }

    String getProxyHost() { return getStringSafe("", defaultConfiguration.proxyHost); }

    String getProxyPassword() { return getStringSafe("", defaultConfiguration.proxyPassword); }

    String getProxySSLCertPath() { return getStringSafe("", defaultConfiguration.proxySSLCertPath); }

    String getProxySSLCertType() { return getStringSafe("", defaultConfiguration.proxySSLCertType); }

    String getProxySSLKeyPassword() { return getStringSafe("", defaultConfiguration.proxySSLKeyPassword); }

    String getProxySSLKeyPath() { return getStringSafe("", defaultConfiguration.proxySSLKeyPath); }

    String getProxySSLKeyType() { return getStringSafe("", defaultConfiguration.proxySSLKeyType); }

    String getProxyUserName() { return getStringSafe("", defaultConfiguration.proxyUserName); }

    String getRegion() { return getStringSafe("", defaultConfiguration.region); }

    String getUserAgent() { return getStringSafe("", defaultConfiguration.userAgent); }

    bool getDisableExpectHeader() { return getBoolSafe("", defaultConfiguration.disableExpectHeader); }

    bool getEnableClockSkewAdjustment() { return getBoolSafe("", defaultConfiguration.enableClockSkewAdjustment); }

    bool getEnableEndpointDiscovery() { return getBoolSafe("", defaultConfiguration.enableEndpointDiscovery); }

    bool getEnableHostPrefixInjection() { return getBoolSafe("", defaultConfiguration.enableHostPrefixInjection); }

    bool getEnableTcpKeepAlive() { return getBoolSafe("", defaultConfiguration.enableTcpKeepAlive); }

    bool getFollowRedirects() { return getBoolSafe("", defaultConfiguration.followRedirects); }

    bool getUseDualStack() { return getBoolSafe("", defaultConfiguration.useDualStack); }

    bool getVerifySSL() { return getBoolSafe("", defaultConfiguration.verifySSL); }

    long getConnectTimeoutMs() { return getBoolSafe("", defaultConfiguration.connectTimeoutMs); }

    long getRequestTimeoutMs() { return getBoolSafe("", defaultConfiguration.requestTimeoutMs); }

    shared_ptr<RateLimiterInterface> getReadRateLimiter() {
        return getStringSafe("", defaultConfiguration.readRateLimiter);
    }

    shared_ptr<RateLimiterInterface> getWriteRateLimiter() {
        return getStringSafe("", defaultConfiguration.writeRateLimiter);
    }

    shared_ptr<Executor> getExecutor() { return getStringSafe("", defaultConfiguration.executor); }

    shared_ptr<RetryStrategy> getRetryStrategy() { return getStringSafe("", defaultConfiguration.retryStrategy); }

    unsigned int getMaxConnections() { return getStringSafe("", defaultConfiguration.maxConnections); }

    unsigned int getProxyPort() { return getStringSafe("", defaultConfiguration.proxyPort); }

    unsigned long getLowSpeedLimit() { return getStringSafe("", defaultConfiguration.lon); }

    unsigned long getTcpKeepAliveIntervalMs() { return getStringSafe("", defaultConfiguration.lon); }
};


#endif //FASTQINDEX_S3CONFIG_H
