/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_FILESOURCE_H
#define FASTQINDEX_FILESOURCE_H

#include "common/IOHelper.h"
#include "process/io/locks/FileLockHandler.h"
#include "process/io/Source.h"
#include <iostream>
#include <fstream>

/**
 * Source implementation for a path object.
 */
class FileSource : public Source {
private:

    path file;

    std::ifstream fStream;

    FileLockHandler lockHandler;

public:

    static shared_ptr<FileSource> from(const path &file) {
        return make_shared<FileSource>(file);
    }

    /**
     * Create an instance of this object with a path source. This will be read by fread and so on.
     * @param file
     */
    explicit FileSource(const path &file);

    ~FileSource() override;

    bool fulfillsPremises() override;

    bool open() override;

    bool openWithReadLock() override;

    bool close() override;

    bool isOpen() override;

    bool hasLock() override;

    bool unlock() override;

    bool eof() override;

    bool isGood() override { return fStream.good(); }

    bool isFile() override { return true; };

    bool isStream() override { return false; };

    bool isSymlink() override { return is_symlink(symlink_status(file)); }

    bool exists() override { return v1::exists(file); };

    int64_t size() override { return exists() ? file_size(file) : 0; }

    bool empty() override { return size() == 0; }

    bool canRead() override { return tell() < size(); }

    bool canWrite() override { return false; }

    path getPath() { return file; }

    string absolutePath() { return file.string(); }

    int64_t read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int64_t seek(int64_t nByte, bool absolute) override;

    int64_t skip(int64_t nBytes) override;

    int64_t rewind(int64_t nByte) override;

    int64_t tell() override;

    int lastError() override;

    string toString() override;
};

#include "Source.h"

#endif //FASTQINDEX_FILESOURCE_H
