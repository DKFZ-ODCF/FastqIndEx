/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/s3/S3Service.h"
#include <UnitTest++/UnitTest++.h>

const char *const S3_SERVICE_TESTS = "Test suite for the S3Config class";

const char *const TEST_CONSTRUCT = "Test construction";

SUITE (S3_SERVICE_TESTS) {

    TEST (TEST_CONSTRUCT) {
        S3ServiceOptions opts;

                CHECK_EQUAL(0, S3Service::getAWSInstanceCount());

        S3Service s0(opts);
                CHECK_EQUAL(1, S3Service::getAWSInstanceCount());
                CHECK(s0.getClient());

        S3Service *s1 = new S3Service(opts);
                CHECK_EQUAL(2, S3Service::getAWSInstanceCount());

        delete s1;
                CHECK_EQUAL(1, S3Service::getAWSInstanceCount());
    }
}