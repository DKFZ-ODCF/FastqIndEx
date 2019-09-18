/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3TESTRUNNER_H
#define FASTQINDEX_S3TESTRUNNER_H

#include "ActualRunner.h"
#include "common/IOHelper.h"
#include "process/io/s3/S3Service.h"
#include "process/io/s3/S3Sink.h"
#include "process/io/s3/S3Source.h"
#include <fstream>

using namespace std;
using namespace std::experimental::filesystem;

class S3TestRunner : public Runner {

    S3Service_S service;

    string bucket;

public:
    S3TestRunner(const S3Service_S &service, const string &bucket) :
            Runner(), service(service), bucket(bucket) {}

protected:
    unsigned char _run() override {
        // Create temp file
        auto fileResult = IOHelper::createTempFile("S3TestRunner");
        if (!fileResult) {
            addErrorMessage("Could not create a temporary file for the S3 test.");
            return 1;
        }

        string ref("ABC\nDEF\n");
        // Prepare temp file, fill in some stuff.
        ofstream stream;
        stream.open(fileResult.result, ios_base::out);
        stream << ref;
        stream.flush();
        stream.close();

        // FQI client
        string s3Object = string("s3://") + bucket + "/" + fileResult.result.filename().string();
        auto fClient = FQIS3Client::from(s3Object, service);

        // Upload temp file
        auto existenceResult = fClient->checkObjectExistence();
        if (!existenceResult || existenceResult.result) {
            addErrorMessage("The S3 test cannot continue. Object: '", s3Object, "' already exists.");
            return 2;
        }

        auto putResult = fClient->putFile(fileResult.result);
        auto existenceRecheckResult = fClient->checkObjectExistence();
        if (!putResult || !existenceRecheckResult) {
            addErrorMessage("Could not upload the temp file '", fileResult.result, "' to '", s3Object, "'");
            return 3;
        }

        auto sizeCheckResult = fClient->getObjectSize();
        if (!sizeCheckResult || sizeCheckResult.result != 8) {
            addErrorMessage("The S3 size check failed for object: '", s3Object, "'.");
            fClient->deleteObjectFromBucket();
            return 4;
        }

        // Download temp file and check contents
        auto source = S3Source::from(s3Object, service);
        source->open();

        Bytef buf[9]{0};
        auto readBytes = source->read(buf, 8);

        string str(reinterpret_cast<char *>(buf));

        if (readBytes != 8 || strcmp(ref.c_str(), str.c_str()) != 0) {
            addErrorMessage("The string of the original file and the downloaded file are not equal!.");
            fClient->deleteObjectFromBucket();
            return 5;
        }

        fClient->deleteObjectFromBucket();

        cerr << "The S3 test succeeded.\n";
        return 0;
    }
};

#endif //FASTQINDEX_S3TESTRUNNER_H
