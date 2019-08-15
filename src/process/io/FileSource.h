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

//    FILE *filePointer{nullptr};

    std::ifstream fStream;

    FileLockHandler lockHandler;

public:

    static shared_ptr<FileSource> from(const path &file) {
        return make_shared<FileSource>(file);
    }

    bool hasLock() override;

    bool unlock() override;

    int64_t rewind(int64_t nByte) override;

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

    bool exists() override { return v1::exists(file); };

    bool isSymlink() override { return is_symlink(symlink_status(file)); }

    bool isRegularFile() {
        if (isSymlink()) {
            return is_regular_file(read_symlink(file));
        } else {
            return is_regular_file(file);
        }
    }

    int64_t size() override { return exists() ? file_size(file) : 0; }

    string absolutePath() { return file.string(); }

    bool isFile() override { return true; };

    bool isStream() override { return false; };

    int64_t read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int64_t seek(int64_t nByte, bool absolute) override;

    int64_t skip(int64_t nBytes) override;

    int64_t tell() override;

    bool canRead() override;

    int lastError() override;

    path getPath() { return file; }

    bool isOpen() override;

    bool eof() override;

    bool isGood() override;

    bool empty() override;

    bool canWrite() override;

    string toString() override;
};

#include "Source.h"

#endif //FASTQINDEX_FILESOURCE_H
