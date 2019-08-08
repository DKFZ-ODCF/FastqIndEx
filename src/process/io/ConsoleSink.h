/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_CONSOLESINK_H
#define FASTQINDEX_CONSOLESINK_H

#include "Sink.h"
#include <string>
#include <cstring>
#include <ostream>
#include <memory>
#include <mutex>

using namespace std;

enum ConsoleSinkType {
    CERR,
    COUT
};

/**
 * This Sink implementation is specific for the console stream cerr and cout
 */
class ConsoleSink : public Sink {
private:

    mutex _mtx;

    ostream *stream;

public:

    static shared_ptr<ConsoleSink> create(ConsoleSinkType type = COUT) {
        return make_shared<ConsoleSink>(type);
    }

    explicit ConsoleSink(ConsoleSinkType type = ConsoleSinkType::COUT) : Sink(true) {
        if (type == CERR)
            this->stream = &std::cerr;
        if (type == COUT)
            this->stream = &std::cout;
    }

    bool hasLock() override {
        return true;
    }

    bool unlock() override {
        return true;
    }

    int64_t rewind(int64_t nByte) override {
        return 0;
    }

    bool openWithWriteLock() override {
        return true;
    }

    bool fulfillsPremises() override {
        return stream == &std::cout || stream == &std::cerr;
    }

    bool open() override {
        return true;//stream != nullptr;
    }

    bool close() override {
        return true;
    }

    bool isOpen() override {
        return true;//stream != nullptr;
    }

    bool eof() override {
        return false;
    }

    bool isGood() override {
        return stream->good();
    }

    bool isFile() override {
        return false;
    }

    bool isStream() override {
        return true;
    }

    bool isSymlink() override {
        return false;
    }

    bool exists() override {
        return true;//stream;
    }

    int64_t size() override {
        return 0;
    }

    bool empty() override {
        return false;
    }

    bool canRead() override {
        return false;
    }

    bool canWrite() override {
        return true;// stream;
    }

    int64_t seek(int64_t nByte, bool absolute) override {
        return 0;
    }

    int64_t skip(int64_t nByte) override {
        return 0;
    }

    string toString() override {
        if (stream == &std::cout)
            return "cout";
        else if (stream == &std::cout)
            return "cerr";
        else
            return "unknown stream type or no stream";
    }

    int64_t tell() override {
        return 0;
    }

    int lastError() override {
        return 0;
    }

    void write(const char *message) override {
        lock_guard<mutex> lock(_mtx);
        if (stream)
            *stream << message;
    }

    void write(const char *message, int len) override {
        lock_guard<mutex> lock(_mtx);
        if (stream)
            stream->write(message, len);
    }

    void write(const string &message) override {
        lock_guard<mutex> lock(_mtx);
        if (stream)
            *stream << message;
    }

    void flush() override {
        lock_guard<mutex> lock(_mtx);
        stream->flush();
    }
};


#endif //FASTQINDEX_SINK_H
