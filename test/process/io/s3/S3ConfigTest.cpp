/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/s3/S3Config.h"
#include "TestResourcesAndFunctions.h"
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/Aws.h>
#include <UnitTest++/UnitTest++.h>

const char *TEST_CONSTRUCT;

const char *S3_CONFIG_TESTS = "Test suite for the S3Config class";

const char *TEST_RESOLVE_CFG_FILES_SET_CREDENTIALS = "Test readAndValidate() with set credentials and available files";
const char *TEST_RESOLVE_CFG_FILES_DEFAULT_CREDENTIALS = "Test readAndValidate() with default credentials";
const char *TEST_RESOLVE_CFG_FILES_MISSING_CREDENTIALS = "Test readAndValidate() with set but missing credentials file";

const char *TEST_RESOLVE_CFG_FILES_SET_CONFIG = "Test readAndValidate() with set config.";
const char *TEST_RESOLVE_CFG_FILES_DEFAULT_CONFIG = "Test readAndValidate() with default config.";
const char *TEST_RESOLVE_CFG_FILES_MISSING_CONFIG = "Test readAndValidate() with missing config.";

const char *const TEST_READ_FILES = "Test readFiles().";
const char *const TEST_FILL_AWSCREDENTIALS = "Test fillAWSCredentials().";
const char *const TEST_FILL_AWSOPTIONS = "Test fillAWSOptions().";

const char *const TEST_SAFE_GET = "Test getAWSAccessKeyId().";
const char *const TEST_GET_AWS_ACCESSKEY_ID = "Test getAWSAccessKeyId().";
const char *const TEST_GET_AWS_SECRET_KEY = "Test getAWSSecretKey().";
const char *const TEST_GET_SOCKET_TIMEOUT = "Test getSocketTimeout().";

SUITE (S3_CONFIG_TESTS) {

    /**
     * Override the original class to reset the credential and config file default locations.
     */
    class S3TestConfig : public S3Config {
    private:
        TestResourcesAndFunctions *res;

    public:
        S3TestConfig(TestResourcesAndFunctions *res, const path &credentialsFile, const path &configFile,
                     const string &selectedConfigSection)
                : res(res), S3Config(S3ServiceOptions(credentialsFile, configFile, selectedConfigSection)) {
            readAndValidate();
        }

        S3TestConfig(TestResourcesAndFunctions *res, const string &credentialsFile, const string &configFile)
                : res(res), S3Config(S3ServiceOptions(TestResourcesAndFunctions::getResource(credentialsFile),
                                                      TestResourcesAndFunctions::getResource(configFile), "default")) {
            readAndValidate();
        }

        path getDefaultAWSCredentialsFile() override {
            return res->filePath("credentials");
        }

        path getDefaultAWSConfigFile() override {
            return res->filePath("config");
        }

        bool readFiles() override {
            return S3Config::readFiles();
        }

    };

    TEST (TEST_CONSTRUCT) {
        S3Config cfg;
                CHECK(!cfg.isValid());
                CHECK(cfg.getConfigFile().empty());
                CHECK(cfg.getCredentialsFile().empty());
                CHECK(cfg.getSelectedConfigSection().empty());
    }

    TEST (TEST_RESOLVE_CFG_FILES_SET_CREDENTIALS) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_RESOLVE_CFG_FILES_SET_CREDENTIALS);

        path credentials = res.createEmptyFile("credentials.txt");
        path config = res.createEmptyFile("config.txt");

        // Also test if the proper config section was set
        S3Config cfg(S3ServiceOptions(credentials, config, "something"));
                CHECK(cfg.getSelectedConfigSection() == "something");
                CHECK(cfg.isValid());
                CHECK(cfg.getCredentialsFile() == credentials);
                CHECK(cfg.getConfigFile() == config);
    }

    TEST (TEST_RESOLVE_CFG_FILES_DEFAULT_CREDENTIALS) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_RESOLVE_CFG_FILES_DEFAULT_CREDENTIALS);

        path credentials = path("");
        path config = path("");

        res.createEmptyFile("credentials");
        res.createEmptyFile("config");

        // Also test if the proper config section was set
        S3TestConfig cfg(&res, credentials, config, "something");
        cfg.readAndValidate();
                CHECK(cfg.getSelectedConfigSection() == "something");
                CHECK(cfg.isValid());
                CHECK(cfg.getCredentialsFile() == cfg.getDefaultAWSCredentialsFile());
                CHECK(cfg.getConfigFile() == cfg.getDefaultAWSConfigFile());
    }

    TEST (TEST_RESOLVE_CFG_FILES_MISSING_CREDENTIALS) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_RESOLVE_CFG_FILES_MISSING_CREDENTIALS);

        path credentials = path("");
        path config = path("");

        res.createEmptyFile("config");

        // Also test if the proper config section was set
        S3TestConfig cfg(&res, credentials, config, "something");
                CHECK(cfg.isValid());
                CHECK(cfg.getCredentialsFile() == cfg.getDefaultAWSConfigFile());
                CHECK(cfg.getConfigFile() == cfg.getDefaultAWSConfigFile());
    }

    TEST (TEST_RESOLVE_CFG_FILES_SET_CONFIG) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_RESOLVE_CFG_FILES_SET_CONFIG);

        path credentials = path("");
        path config = res.createEmptyFile("testConf");

        // Also test if the proper config section was set
        S3TestConfig cfg(&res, credentials, config, "something");
                CHECK(cfg.isValid());
                CHECK(cfg.getCredentialsFile() == config);
                CHECK(cfg.getConfigFile() == config);
    }

    TEST (TEST_RESOLVE_CFG_FILES_DEFAULT_CONFIG) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_RESOLVE_CFG_FILES_DEFAULT_CONFIG);

        path credentials = path("");
        path config = path("");

        res.createEmptyFile("config");

        // Also test if the proper config section was set
        S3TestConfig cfg(&res, credentials, config, "something");
                CHECK(cfg.isValid());
                CHECK(cfg.getCredentialsFile() == cfg.getDefaultAWSConfigFile());
                CHECK(cfg.getConfigFile() == cfg.getDefaultAWSConfigFile());
    }

    TEST (TEST_RESOLVE_CFG_FILES_MISSING_CONFIG) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_RESOLVE_CFG_FILES_MISSING_CONFIG);

        path credentials = path("");
        path config = path("");

        // Also test if the proper config section was set
        S3TestConfig cfg(&res, credentials, config, "something");
                CHECK(!cfg.isValid());
                CHECK(cfg.getCredentialsFile().empty());
                CHECK(cfg.getConfigFile().empty());
    }

    TEST (TEST_READ_FILES) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_READ_FILES);

        S3TestConfig cfg(&res, "awsCredentialsExample", "awsConfigExample");
                CHECK(cfg.getAWSAccessKeyId() == "aaa");
                CHECK(cfg.getAWSSecretKey() == "bbb");
                CHECK(cfg.getConnectTimeoutMs() == 10000);
        // Now default values

                CHECK(false);
    }

    TEST (TEST_FILL_AWSCREDENTIALS) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_READ_FILES);

        S3TestConfig cfg(&res, "awsCredentialsExample", "awsConfigExample");
        Aws::Auth::AWSCredentials credentials;
        cfg.fillAWSCredentials(credentials);
    }

    TEST (TEST_FILL_AWSOPTIONS) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_READ_FILES);

        S3TestConfig cfg(&res, "awsCredentialsExample", "awsConfigExample");
        Aws::Client::ClientConfiguration configuration;
        cfg.fillAWSClientConfiguration(configuration);

                CHECK(configuration.proxyHost == "someproxy");
                CHECK(configuration.endpointOverride == "https://a.nice.s3.host.com");
                CHECK(configuration.proxyPort == 3128);
    }

    TEST (TEST_SAFE_GET) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_GET_AWS_ACCESSKEY_ID);

        S3TestConfig cfg(&res, "awsCredentialsExample", "awsConfigExample");
    }

    TEST (TEST_GET_AWS_ACCESSKEY_ID) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_GET_AWS_ACCESSKEY_ID);

        S3TestConfig cfg(&res, "awsCredentialsExample", "awsConfigExample");
    }

    TEST (TEST_GET_AWS_SECRET_KEY) {
        TestResourcesAndFunctions res(S3_CONFIG_TESTS, TEST_GET_AWS_SECRET_KEY);

        S3TestConfig cfg(&res, "awsCredentialsExample", "awsConfigExample");
    }
}