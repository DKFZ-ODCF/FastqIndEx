/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_IOHELPER_H
#define FASTQINDEX_IOHELPER_H

#include "../process/extract/IndexReader.h"
#include "../process/io/InputSource.h"
#include "../process/io/PathInputSource.h"
#include "CommonStructsAndConstants.h"
#include "ErrorAccumulator.h"
#include <cstdio>
#include <experimental/filesystem>
#include <map>
#include <zlib.h>

using namespace std;
using namespace std::experimental::filesystem;

class IOHelper {

private:

    static void report(const stringstream &sstream, ErrorAccumulator* errorAccumulator);

public:

    static path getUserHomeDirectory();

    /**
     * Check file existence and read / write capability for it. Report to either cerr or the pointed to errorAccumulator
     * instance.
     *
     * @param file              The file to check
     * @param fileType        Some identifier, like "credentials".
     *                          Will be placed into "The '<identifier>' file '<path>'... messages
     * @param errorAccumulator  If nullptr is set here, cerr will be used to report errors. Otherwise, the instance will
     *                          store the new message.
     * @return                  true, if the file is valid.
     */
    static bool checkFileReadability(const path &file, const string & fileType, ErrorAccumulator* errorAccumulator);

    static shared_ptr<map<string, string>> loadIniFile(path file, string section);
};


#endif //FASTQINDEX_IOHELPER_H
