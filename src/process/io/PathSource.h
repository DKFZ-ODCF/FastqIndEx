/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_PATHSOURCE_H
#define FASTQINDEX_PATHSOURCE_H

#include "common/IOHelper.h"
#include "process/io/locks/PathLockHandler.h"
#include "process/io/Source.h"
#include <iostream>
#include <fstream>

/**
 * Source implementation for a path object.
 */
class PathSource : public Source {
private:

    path file;

//    FILE *filePointer{nullptr};

    std::ifstream fStream;

    PathLockHandler lockHandler;

public:

    static shared_ptr<PathSource> from(const path &file) {
        return make_shared<PathSource>(file);
    }

    bool hasLock() override;

    bool unlock() override;

    int rewind(uint64_t nByte) override;

    /**
     * Create an instance of this object with a path source. This will be read by fread and so on.
     * @param file
     */
    explicit PathSource(const path &file);

    virtual ~PathSource();

    bool fulfillsPremises() override;

    bool open() override;

    bool openWithReadLock() override;

    bool close() override;

    bool exists() override { return v1::exists(file); };

    bool isSymlink() { return is_symlink(symlink_status(file)); }

    bool isRegularFile() {
        if (isSymlink()) {
            return is_regular_file(read_symlink(file));
        } else {
            return is_regular_file(file);
        }
    }

    uint64_t size() override { return exists() ? file_size(file) : 0; }

    string absolutePath() { return file.string(); }

    bool isFile() override { return true; };

    bool isStream() override { return false; };

    int read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int seek(int64_t nByte, bool absolute) override;

    int skip(uint64_t nBytes) override;

    uint64_t tell() override;

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

#endif //FASTQINDEX_PATHSOURCE_H
