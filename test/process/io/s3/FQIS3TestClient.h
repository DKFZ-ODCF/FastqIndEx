/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#pragma once

#include "process/io/s3/S3Service.h"
#include "process/io/s3/FQIS3Client.h"
#include "TestResourcesAndFunctions.h"
#include <experimental/filesystem>

using namespace std::experimental::filesystem;

/**
 * Instead of spawning an instance of our S3 helper application, we invoke the cat command and use this to write a file
 * to our named pipe.
 */
class TestS3GetObjectProcessWrapper : public S3GetObjectProcessWrapper {
protected:
    void processThreadFunc() override {
        stringstream sstream;
        sstream << "cat '" << s3Object << "' > '" << fifo.string() << "'";
        TestResourcesAndFunctions::runCommand(sstream.str());
    }

public:

    TestS3GetObjectProcessWrapper(
            const S3ServiceOptions &serviceOptions,
            const path &fifo,
            const string &s3FakeObject,
            int64_t readStart
    ) : S3GetObjectProcessWrapper(serviceOptions, fifo, s3FakeObject, readStart) {}
};

/**
 * Test implementation of FQIS3Client which works on a file instead of using an S3 server/service.
 */
class FQIS3TestClient : public FQIS3Client {
private:

    path s3FakeObject;

public:

    static shared_ptr<FQIS3TestClient> from(const string &s3FakeObject, const S3Service_S &service) {
        return make_shared<FQIS3TestClient>(s3FakeObject, service);
    }

    FQIS3TestClient(const string &s3FakeObject, const S3Service_S &service)
            : FQIS3Client(service) {
        this->s3FakeObject = s3FakeObject;
        this->bucketName = "TestBucket";
        this->objectName = this->s3FakeObject.filename().string();
        this->s3Path = string("s3://") + this->bucketName + "/" + this->objectName;
    }

    bool isValid() override {
        return exists(s3FakeObject) && IOHelper::checkFileReadability(s3FakeObject, "S3 fake file", nullptr);
    }

    Result<list<S3Object>> getObjectList() override {
        list<S3Object> list;
        bool valid = isValid();
        if (valid) {
            S3Object object(s3FakeObject.filename().string(), file_size(s3FakeObject));
            list.emplace_back(object);
        }
        return {valid, list};
    }

    Result<uint64_t> getObjectSize() override {
        if (isValid())
            return {true, file_size(s3FakeObject)};
        return {false, 0};
    }

    S3GetObjectProcessWrapper_S createS3GetObjectProcessWrapper(const path &fifo, u_int64_t readStart) override {
        return shared_ptr<S3GetObjectProcessWrapper>(
                new TestS3GetObjectProcessWrapper(
                        this->getS3ServiceOptions(),
                        fifo,
                        this->s3Path,
                        readStart
                )
        );
    }

    FQIS3ClientRequestBooleanResult putFile(const string &file) override {
        return FQIS3Client::putFile(file);
    }
};

typedef shared_ptr<FQIS3TestClient> FQIS3TestClient_S;