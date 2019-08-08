/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/StringHelper.h"
#include "S3Source.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>

S3Source::S3Source(const string &s3Path, const S3ServiceOptions &s3ServiceOptions) :
        fqiS3Client(s3Path, s3ServiceOptions) {
}

S3Source::~S3Source() {
    close();
}

void S3Source::setReadStart(int64_t startBytes) {
    if (isOpen()) {
        always("Reset S3Source with new Range: [", to_string(startBytes), "-", to_string(size()), "]");
        close();
        readStart = startBytes;
        openWithReadLock();
    } else
        readStart = startBytes;
}

bool S3Source::open() {

    if (_isOpen)
        return true;

    // Create fifo
    auto[success, fifo] = IOHelper::createTempFifo("FASTQIndEx_S3SourceFIFO");
    if (!success) {
        ErrorAccumulator::addErrorMessage("Could not create a fifo for the S3 source '", fqiS3Client.getS3Path(), "'.");
        return false;
    }

    this->fifo = fifo;
    auto s3 = S3Service::getInstance();

    // Create and start asynchronous request
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(fqiS3Client.getBucketName().c_str());
    object_request.SetKey(fqiS3Client.getObjectName().c_str());

    if (readStart != 0) {
        auto rangeString = "bytes=" + to_string(readStart) + "-" + to_string(size());
        object_request.SetRange(rangeString.c_str());
    }
    string object = fqiS3Client.getObjectName();
    object_request.SetResponseStreamFactory([&]() {
        always("Writing ", fqiS3Client.getObjectName(), " with S3 to ", this->fifo);
        auto stream = Aws::New<FStream>(fqiS3Client.getObjectName().c_str(), this->fifo.c_str(),
                                        std::ios_base::out | std::ios_base::binary);
        this->s3FStream = stream;
        return stream;
    });

    auto context = Aws::MakeShared<Aws::Client::AsyncCallerContext>("GetObjectAllocationTag");
    context->SetUUID(fqiS3Client.getObjectName().c_str());
    auto client = s3->getClient();
    s3->getClient()->GetObjectAsync(object_request, get_object_async_finished, context);
    // Will wait until stream was produced.

    this->stream.open(this->fifo, ios_base::in | ios_base::binary);
    this->streamSource = StreamSource::from(&this->stream);

    _isOpen = true;
    return true;
}

bool S3Source::close() {
    if (!_isOpen)
        return true;

    if (this->s3FStream->is_open()) {
        this->s3FStream->close();
        delete this->s3FStream;
        this->s3FStream = nullptr;
    }
    this->stream.close();

    if (this->streamSource.get())
        this->streamSource->close();
    this->streamSource.reset();

    if (std::experimental::filesystem::exists(this->fifo)) {
        remove(fifo);
        this->fifo = "";
    }

    _isOpen = false;
    return true;
}

int64_t S3Source::read(Bytef *targetBuffer, int numberOfBytes) {
    return this->streamSource->read(targetBuffer, numberOfBytes);
}

int S3Source::readChar() {
    return this->streamSource->readChar();
}

int64_t S3Source::seek(int64_t nByte, bool absolute) {
    if (absolute) {
        return this->streamSource->seek(nByte - readStart, true);
    } else
        return this->streamSource->seek(nByte, false);
}

int64_t S3Source::skip(int64_t nBytes) {
    return streamSource->seek(nBytes, false);
}

int64_t S3Source::tell() {
    return readStart + streamSource->tell();
}

bool S3Source::canRead() {
    return streamSource->canRead();//tell() < size();
}

int S3Source::lastError() {
    return 0;
}

bool S3Source::fulfillsPremises() {

    if (!fqiS3Client.isValid()) {
        return false;
    }

    auto result = fqiS3Client.checkObjectExistence();
    if (!result.requestWasSuccessful) {
        ErrorAccumulator::addErrorMessage("Could not check for file '", fqiS3Client.getS3Path(), "'");
        return false;
    }

    if (!result.result) {
        ErrorAccumulator::addErrorMessage("The file '", this->fqiS3Client.getS3Path(), "' does not exist. ");
        return false;
    }
    return true;
}

bool S3Source::isOpen() {
    return _isOpen;
}

bool S3Source::eof() {
    return tell() >= size();
}

bool S3Source::isGood() {
    return true;//streamSource->isGood();
}

bool S3Source::empty() {
    return size() <= 0;
}

bool S3Source::canWrite() {
    return false;
}

string S3Source::toString() {
    return fqiS3Client.getS3Path();
}

bool S3Source::openWithReadLock() {
    return open();
}

bool S3Source::hasLock() {
    return true;
}

bool S3Source::unlock() {
    return true;
}

int64_t S3Source::getTotalReadBytes() {
    return Source::getTotalReadBytes();
}

int64_t S3Source::rewind(int64_t nByte) {
    return streamSource->rewind(nByte);
}

const path &S3Source::getFifo() const {
    return fifo;
}
