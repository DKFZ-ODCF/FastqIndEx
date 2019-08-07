/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_IOHELPER_H
#define FASTQINDEX_IOHELPER_H

#include "ErrorAccumulator.h"
#include <cstdio>
#include <experimental/filesystem>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <zlib.h>

using namespace std;
using namespace std::experimental::filesystem;


class IOHelper {

private:

    static recursive_mutex iohelper_mtx;

public:

    /**
     * Append the message stored in sstream to the errorAccumulator OR write it to cerr, if the errorAccumulator is null
     * @param sstream           The stringstream containing an error message.
     * @param errorAccumulator  An errorAccumulator instance OR nullptr.
     */
    static void report(const stringstream &sstream, ErrorAccumulator *errorAccumulator);

    static path getUserHomeDirectory();

    /**
     * Try to create a temporary file
     * @return A tuple indicating [success, temp directory path]
     */
    static tuple<bool, path> createTempDir(const string &prefix);

    /**
     * Try to create a temporary file
     * @return A tuple indicating [success, temp file path]
     */
    static tuple<bool, path> createTempFile(const string &prefix);

    /**
     * Create a unique fifo in the temp folder.
     * Needs to be cleaned before the application exits.
     * @return A tuple indicating [success, fifo path]
     */
    static tuple<bool, path> createTempFifo(const string &prefix);

    /**
     * Check file existence and read / write capability for it. Report to either cerr or the pointed to errorAccumulator
     * instance.
     *
     * @param file              The file to check
     * @param fileType          Some identifier, like "credentials".
     *                          Will be placed into "The '<identifier>' file '<path>'... messages
     * @param errorAccumulator  If nullptr is set here, cerr will be used to report errors. Otherwise, the instance will
     *                          store the new message.
     * @return                  true, if the file is valid.
     */
    static bool checkFileReadability(const path &file, const string &fileType, ErrorAccumulator *errorAccumulator);

    /**
     * Returns the full path of file.
     * E.g. with the path "file.txt" and the working directory "/tmp", this will e.g. return "/tmp/file.txt"
     */
    static path fullPath(const path &file);

    static shared_ptr<map<string, string>> loadIniFile(path file, string section);
};

#endif //FASTQINDEX_IOHELPER_H
