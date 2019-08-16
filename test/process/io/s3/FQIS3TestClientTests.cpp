/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/s3/S3Source.h"
#include "process/io/s3/FQIS3TestClient.h"
#include "TestConstants.h"
#include "TestResourcesAndFunctions.h"
#include <UnitTest++/UnitTest++.h>


SUITE (S3_FQITESTSOURCE_TEST_SUITE) {
    const char *const S3_FQITESTSOURCE_TEST_SUITE = "Test suite for the FQIS3TestClient class";
    const char *const S3_FQITESTSOURCE_ISVALID = "Test isValid";
    const char *const S3_FQITESTSOURCE_GETOBJECTLIST = "Test getObjectList";
    const char *const S3_FQITESTSOURCE_GETOBJECTSIZE = "Test getObjectSize";

//shared_ptr<S3Source> createTestSourceWithBackingFile(const path &file) {
//    auto client = make_shared<FQIS3TestClient>(file, S3Service::getDefault());
//    return S3Source::from(client);
//}

    FQIS3TestClient_S createTestClient(const string &filename) {
        auto file = TestResourcesAndFunctions::getResource(filename);
        return make_shared<FQIS3TestClient>(file, S3Service::getDefault());
    }

    TEST (S3_FQITESTSOURCE_ISVALID) {
        auto c1 = createTestClient(TEST_FASTQ_SMALL);
                CHECK(c1->isValid());

        auto c2 = createTestClient("blablabla");
                CHECK(!c2->isValid());
    }

    TEST (S3_FQITESTSOURCE_GETOBJECTLIST) {
        auto c1 = createTestClient(TEST_FASTQ_SMALL);
        auto r1 = c1->getObjectList();
                CHECK(r1);
                CHECK_EQUAL(1U, r1.result.size());
        auto object = r1.result;
                CHECK_EQUAL(TEST_FASTQ_SMALL, object.begin()->name);

        auto exptectedSize = file_size(TestResourcesAndFunctions::getResource(TEST_FASTQ_SMALL));
                CHECK_EQUAL(exptectedSize, object.begin()->size);

        auto c2 = createTestClient("blablabla");
        auto r2 = c2->getObjectList();
                CHECK(!r2);
                CHECK_EQUAL(0U, r2.result.size());
    }

    TEST (S3_FQITESTSOURCE_GETOBJECTSIZE) {
        vector<string> files{TEST_FASTQ_SMALL, TEST_FASTQ_LARGE, "blabla"};
        vector<bool> success{true, true, false};

        for (uint i = 0; i < files.size(); i++) {
            auto c1 = createTestClient(files[i]);
            auto r1 = c1->getObjectSize();
                    CHECK_EQUAL(success[i], r1);

            if (!r1)  // The result might contain garbage, if the query was not successful.
                continue;
            auto expectedSize = file_size(TestResourcesAndFunctions::getResource(files[i]));
            auto actualSize = r1.result;
                    CHECK_EQUAL(expectedSize, actualSize);
        }
    }

    TEST(S3_FQITESTSOURCE_CREATES3GETOBJECTWRAPPER) {
        // createS3GetObjectProcessWrapper
    }
}