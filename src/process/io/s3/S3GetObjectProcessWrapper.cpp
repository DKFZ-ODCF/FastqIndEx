/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/CommonStructsAndConstants.h"
#include "S3GetObjectProcessWrapper.h"

void S3GetObjectProcessWrapper::processThreadFunc() {
    stringstream cmd;
    // We  need to pass double ticks with a whitespace in it (" ") if you have an empty argument.
    // "" and '' were not regarded as parameters in our tests.
    cmd << "\"" << S3HELPER_BINARY << "\" \""
        << this->fifo.string() << "\" \""
        << this->s3Object << "\" \""
        << this->serviceOptions.configSection << "\" \""
        << this->serviceOptions.configFile.string() << " \" \""
        << this->serviceOptions.credentialsFile.string() << " \" \""
        << this->readStart << "\""; // Single ticks will be regarded as two parameters with a space inside...
    ErrorAccumulator::debug("Async call: ", cmd.str());
    int result = system(cmd.str().c_str());
    ErrorAccumulator::debug("Async call to S3 started with result of: ", to_string(result));
}

void S3GetObjectProcessWrapper::start() {
    lock_guard<mutex> lock(mtx);
    if (!processThread) {
        processThread = make_shared<thread>(&S3GetObjectProcessWrapper::processThreadFunc, this);
    }
}

void S3GetObjectProcessWrapper::waitForFinish() {
    lock_guard<mutex> lock(mtx); // Prevent a new process from being started
    if (processThread) {
        processThread->join();
        processThread.reset();
    }
}
