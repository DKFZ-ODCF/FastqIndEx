/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_PATHINPUTSOURCE_H
#define FASTQINDEX_PATHINPUTSOURCE_H

#include "InputSource.h"
#include <iostream>
#include <fstream>

/**
 * InputSource implementation for a path object.
 */
class PathInputSource : public InputSource {
private:

    path source;

    FILE *filePointer{nullptr};

    std::ifstream fPointer;

public:

    /**
     * Create an instance of this object with a path source. This will be read by fread and so on.
     * @param source
     */
    explicit PathInputSource(const path &source);

    virtual ~PathInputSource();

    bool open() override;

    bool close() override;

    bool exists() override { return v1::exists(source); };

    bool isSymlink() { return is_symlink(symlink_status(source)); }

    bool isRegularFile() {
        if (isSymlink()) {
            return is_regular_file(read_symlink(source));
        } else {
            return is_regular_file(source);
        }
    }

    uint64_t size() override { return file_size(source); }

    string absolutePath() { return source.string(); }

    bool isFileSource() override { return true; };

    bool isStreamSource() override { return false; };

    int read(Bytef *targetBuffer, int numberOfBytes) override;

    int readChar() override;

    int seek(int64_t nByte, bool absolute) override;

    int skip(uint64_t nBytes) override;

    uint64_t tell() override;

    bool canRead() override;

    int lastError() override;

    path getPath() { return source; }

};

#include "InputSource.h"

#endif //FASTQINDEX_PATHINPUTSOURCE_H
