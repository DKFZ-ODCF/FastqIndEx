/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_RESULT_H
#define FASTQINDEX_RESULT_H

#include <string>

using namespace std;

template<class R>
class Result {
public:
    const bool success;

    const R result;

    const string message;

    Result(const bool success, const R &result, const string &message = "")
            : success(success), result(result), message(message) {}


    operator bool() const {
        return success;
    }
};

#endif //FASTQINDEX_RESULT_H
