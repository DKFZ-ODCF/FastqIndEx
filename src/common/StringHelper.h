/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_StringHelper_H
#define FASTQINDEX_StringHelper_H

#include <vector>
#include <sstream>
#include <string>

using namespace std;

class StringHelper {

public:

    static vector<string> splitStr(const string &str, char delimiter = '\n');
};


#endif //FASTQINDEX_StringHelper_H
