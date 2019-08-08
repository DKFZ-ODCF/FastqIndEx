/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3CONFIG_H
#define FASTQINDEX_S3CONFIG_H

#include "common/ErrorAccumulator.h"
#include "common/IOHelper.h"
#include "process/io/s3/S3ServiceOptions.h"
#include <aws/core/client/AWSClient.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/ClientConfiguration.h>
#include <experimental/filesystem>

using namespace Aws;
using namespace Aws::Client;
using namespace Aws::Http;
using namespace Aws::Utils;
using namespace Aws::Utils::RateLimits;
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

    S3Config() = default;

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
    explicit S3Config(const S3ServiceOptions &s3Opts);

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

    bool hasOptionSet(const string &key) {
        auto iterator = allValues.find(key);
        bool has = allValues.end() != iterator;
        //allValues.erase(iterator); // According to the cppreference example, BUT caused exceptions!
        return has;
    }

    string getStringSafe(const string &key, string _default = "") {
        if (!hasOptionSet(key))
            return _default;
        return allValues[key];
    }

    int getIntSafe(const string &key, int _default = 0) {
        string value = getStringSafe(key, to_string(_default));
        int result = _default;
        try {
            result = stoi(allValues[key]);
        } catch (const invalid_argument &e) {
            addErrorMessage("'", value, "' cannot be converted to an integer.");
        } catch (const out_of_range &e) {
            addErrorMessage("'", value, "' is too large for an integer conversion.");
        }
        return result;
    }

    uint getUIntSafe(const string &key, uint _default = 0) {
        string value = getStringSafe(key, to_string(_default));
        uint result = _default;
        try {
            // Unfortunately, C++ does not offer the stoui method (or stou). Why? Nobody knows. I found
            // the following code snippet here: https://stackoverflow.com/questions/8715213/why-is-there-no-stdstou

            unsigned long intermediate = std::stoul(allValues[key]);
            if (result > std::numeric_limits<unsigned>::max()) {
                throw std::out_of_range("stou");
            } else {
                result = static_cast<uint>(intermediate);
            }
            return result;
        } catch (const invalid_argument &e) {
            addErrorMessage("'", value, "' cannot be converted to an unsigned integer.");
        } catch (const out_of_range &e) {
            addErrorMessage("'", value, "' is too large for an unsigned integer conversion.");
        }
        return result;
    }

    long getLongSafe(const string &key, long _default = 0) {
        string value = getStringSafe(key, to_string(_default));
        long result = _default;
        try {
            result = stol(allValues[key]);
        } catch (const invalid_argument &e) {
            addErrorMessage("'", value, "' cannot be converted to a long.");
        } catch (const out_of_range &e) {
            addErrorMessage("'", value, "' is too large for a long conversion.");
        }
        return result;
    }

    bool getBoolSafe(const string &key, bool _default = false) {
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

        } catch (const invalid_argument &e) {
            addErrorMessage("'", value, "' cannot be converted to bool. Must be either [T/t]rue or [F/f]alse.");
        }
        return result;
    }

    Scheme getSchemeSafe(const string &key, Scheme _default) {
        string value = getStringSafe(key, _default == Scheme::HTTPS ? "https" : "http");
        string str = value;
        Scheme result = _default;
        try {
            transform(str.begin(), str.end(), str.begin(), ::tolower);
            if (str == "https")
                result = Scheme::HTTPS;
            else if (str == "http")
                result = Scheme::HTTP;
            else
                throw invalid_argument("");
        } catch (const invalid_argument &e) {
            addErrorMessage("'", value, "' is not a valid scheme, must be either Http or Https.");
        }
        return result;
    }

    string getAWSAccessKeyId();

    string getAWSSecretKey();


//    int getSocketTimeout() { return getIntSafe("socket_timeout", defaultConfiguration.connectTimeoutMs); }

    Scheme getProxyScheme() {
        return getSchemeSafe("", defaultConfiguration.proxyScheme);
    }

    Scheme getScheme() { return getSchemeSafe("use_https", defaultConfiguration.scheme); }

//    TransferLibType getHttpLibOverride() {
//        return defaultConfiguration.httpLibOverride;
//    }

    String getCaFile() { return String(getStringSafe("ca_certs_file", string(defaultConfiguration.caFile))); }

//    String getCaPath() { return getStringSafe("", defaultConfiguration.caPath); }

    String getEndpointOverride() {
        return String(getStringSafe("host_base", string(defaultConfiguration.endpointOverride)));
    }

    String getProxyHost() { return String(getStringSafe("proxy_host", string(defaultConfiguration.proxyHost))); }

//    String getProxyPassword() { return getStringSafe("", defaultConfiguration.proxyPassword); }

//    String getProxySSLCertPath() { return getStringSafe("", defaultConfiguration.proxySSLCertPath); }

//    String getProxySSLCertType() { return getStringSafe("", defaultConfiguration.proxySSLCertType); }

//    String getProxySSLKeyPassword() { return getStringSafe("", defaultConfiguration.proxySSLKeyPassword); }

//    String getProxySSLKeyPath() { return getStringSafe("", defaultConfiguration.proxySSLKeyPath); }

//    String getProxySSLKeyType() { return getStringSafe("", defaultConfiguration.proxySSLKeyType); }

//    String getProxyUserName() { return getStringSafe("", defaultConfiguration.proxyUserName); }

    String getRegion() { return String(getStringSafe("bucket_location", string(defaultConfiguration.region))); }

    String getUserAgent() { return String(string(defaultConfiguration.userAgent)); }

//    bool getDisableExpectHeader() { return getBoolSafe("", defaultConfiguration.disableExpectHeader); }

//    bool getEnableClockSkewAdjustment() { return getBoolSafe("", defaultConfiguration.enableClockSkewAdjustment); }

//    bool getEnableEndpointDiscovery() { return getBoolSafe("", defaultConfiguration.enableEndpointDiscovery); }

//    bool getEnableHostPrefixInjection() { return getBoolSafe("", defaultConfiguration.enableHostPrefixInjection); }

//    bool getEnableTcpKeepAlive() { return getBoolSafe("", defaultConfiguration.enableTcpKeepAlive); }

    bool getFollowRedirects() { return getBoolSafe("follow_symlinks", defaultConfiguration.followRedirects); }

//    bool getUseDualStack() { return getBoolSafe("", defaultConfiguration.useDualStack); }

    bool getVerifySSL() { return getBoolSafe("check_ssl_certificate", defaultConfiguration.verifySSL); }

    long getConnectTimeoutMs() { return getLongSafe("socket_timeout", defaultConfiguration.connectTimeoutMs); }

//    long getRequestTimeoutMs() { return getBoolSafe("", defaultConfiguration.requestTimeoutMs); }

//    shared_ptr<RateLimiterInterface> getReadRateLimiter() {
//        return defaultConfiguration.readRateLimiter;
//    }

//    shared_ptr<RateLimiterInterface> getWriteRateLimiter() {
//        return defaultConfiguration.writeRateLimiter;
//    }

//    shared_ptr<Executor> getExecutor() { defaultConfiguration.executor; }

//    shared_ptr<RetryStrategy> getRetryStrategy() {
//        return defaultConfiguration.retryStrategy;
//    }

//    unsigned int getMaxConnections() { return getIntSafe("", defaultConfiguration.maxConnections); }

    unsigned int getProxyPort() { return getUIntSafe("proxy_port", defaultConfiguration.proxyPort); }

//    unsigned long getLowSpeedLimit() { return getLongSafe("", defaultConfiguration.lowSpeedLimit); }

//    unsigned long getTcpKeepAliveIntervalMs() { return getLongSafe("", defaultConfiguration.tcpKeepAliveIntervalMs); }
};


#endif //FASTQINDEX_S3CONFIG_H
