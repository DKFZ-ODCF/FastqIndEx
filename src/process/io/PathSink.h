/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_PATHSINK_H
#define FASTQINDEX_PATHSINK_H

#include "Sink.h"
#include "common/IOHelper.h"
#include "process/io/locks/LockHandler.h"
#include "process/io/locks/PathLockHandler.h"
#include <experimental/filesystem>
#include <fstream>
#include <unistd.h>

using namespace std;
using namespace std::experimental::filesystem;

class PathSink : public Sink {

private:

    path file;

    std::fstream fStream;

    PathLockHandler lockHandler;

public:

    static shared_ptr<PathSink> from(const path &file, bool forceOverwrite = false) {
        return make_shared<PathSink>(file, forceOverwrite);
    }

    explicit PathSink(const path &file, bool forceOverwrite = false);

    bool fulfillsPremises() override;

    bool open() override;

    bool openWithWriteLock() override;

    path getPath() {
        return file;
    }

    path parentPath() {
        return file.parent_path();
    }

    void write(const char *message) override;

    void write(const char *message, int streamSize) override;

    void write(const string &message) override;

    void flush() override;

    bool close() override;

    bool isOpen() override;

    bool eof() override;

    bool isGood() override;

    bool isFile() override;

    bool isStream() override;

    bool isSymlink() override;

    bool exists() override;

    int64_t size() override;

    bool empty() override;

    bool canRead() override;

    bool canWrite() override;

    int64_t seek(int64_t nByte, bool absolute) override;

    int64_t skip(int64_t nByte) override;

    string toString() override;

    int64_t tell() override;

    int lastError() override;

    vector<string> getErrorMessages() override;

    bool hasLock() override;

    bool unlock() override;

    int64_t rewind(int64_t nByte) override;
};

#endif //FASTQINDEX_PATHSINK_H
