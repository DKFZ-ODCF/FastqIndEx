/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "CommonStructsAndConstants.h"
#include "StringHelper.h"
#include <regex>

vector<string> StringHelper::splitStr(const string &str, char delimiter) {
    stringstream ss(str);
    string item;
    vector<string> splittedStrings;
    while (getline(ss, item, delimiter)) {
        splittedStrings.push_back(item);
    }

    return splittedStrings;
}

int64_t StringHelper::parseStringValue(const string &str) {
    if (str == "-1")
        return -1;

    long long result = 0;
    if (regex_match(str.c_str(), regex("[0-9]+[kmgtKMGT]"))) {
        result = stoll(str.substr(0, str.length() - 1));
        char unit = static_cast<char>(tolower(str.c_str()[str.length() - 1]));
        if (unit == 'k') {
            result *= kB;
        } else if (unit == 'm') {
            result *= MB;
        } else if (unit == 'g') {
            result *= GB;
        } else if (unit == 't') {
            result *= TB;
        }
    } else if (std::regex_match(str.c_str(), regex("[0-9]+"))) {
        result = stoll(str) * MB;
    }

    if (result <= 0) return -1;

    return result;
}
