/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_FILESINK_H
#define FASTQINDEX_FILESINK_H

#include "Sink.h"
#include "common/IOHelper.h"
#include "process/io/locks/LockHandler.h"
#include "process/io/locks/FileLockHandler.h"
#include <experimental/filesystem>
#include <fstream>
#include <unistd.h>

using namespace std;
using namespace std::experimental::filesystem;

class FileSink : public Sink {

private:

    path file;

    std::fstream fStream;

    FileLockHandler lockHandler;

public:

    static shared_ptr<FileSink> from(const path &file, bool forceOverwrite = false) {
        return make_shared<FileSink>(file, forceOverwrite);
    }

    explicit FileSink(const path &file, bool forceOverwrite = false);

    bool fulfillsPremises() override;

    bool open() override;

    bool openWithWriteLock() override;

    bool close() override;

    bool isOpen() override;

    bool hasLock() override;

    bool unlock() override;

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

    path getPath() { return file; }

    path parentPath() { return file.parent_path(); }

    void write(const char *message) override;

    void write(const char *message, int streamSize) override;

    void write(const string &message) override;

    void flush() override;

    int64_t seek(int64_t nByte, bool absolute) override;

    int64_t skip(int64_t nByte) override;

    int64_t rewind(int64_t nByte) override;

    int64_t tell() override;

    int lastError() override;

    vector<string> getErrorMessages() override;

    string toString() override;
};

#endif //FASTQINDEX_FILESINK_H
