/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_ERRORACCUMULATOR_H
#define FASTQINDEX_ERRORACCUMULATOR_H

#include <iostream>
#include <string>
#include <vector>

using std::vector;
using std::string;


/**
 * The class is used to provide basic error storage methods.
 */
class ErrorAccumulator {
    typedef const string &_cstr;

private:

    /**
     * Keep a list of errors which came up during fulfillsPremises()
     */
    vector<string> errorMessages;

    static int verbosity;

public:

    virtual ~ErrorAccumulator() = default;

    /**
     * Set the runners verbosity level in a range from 0 to 3 IF and only IF the passed value is in this range.
     *
     * 0 == severe messages and important messages only
     * 1 == warnings as well
     * 2 == informational messages
     * 3 == debug messages
     *
     * @param verbosity
     */
    static void setVerbosity(int verbosity);

    static bool verbosityIsSetToDebug();

    static void always(_cstr s0, _cstr s1 = "", _cstr s2 = "", _cstr s3 = "", _cstr s4 = "", _cstr s5 = "");

    static void debug(_cstr s0, _cstr s1 = "", _cstr s2 = "", _cstr s3 = "", _cstr s4 = "", _cstr s5 = "");

    static void info(const string &msg);

    static void warning(const string &msg);

    static void severe(const string &msg);

    virtual vector<string> getErrorMessages();

    /**
     * This would actually be a perfect example for a variadic function but handling variadic functions is tricky as
     * the 'va_...()' macros don't know about the number of passed arguments.
     */
    void addErrorMessage(_cstr s0, _cstr s1 = "", _cstr s2 = "", _cstr s3 = "", _cstr s4 = "", _cstr s5 = "");

    static string join(_cstr s0, _cstr s1 = "", _cstr s2 = "", _cstr s3 = "", _cstr s4 = "", _cstr s5 = "");

    /**
     * This method can be used, if two vectors should be merged. Note, that we always copy the content of the two source
     * vectors into the new vector. However, as this is method is mostly intended to be used with the ErrorAccumulator
     * messages and these are only requested and, when errors came up, we'll accept the price of copy.
     * @param l The left vector
     * @param r The right vector
     * @return A new vector<string> with both vectors merged. The messages of l will be placed before the messages of r.
     */
    static vector<string> mergeToNewVector(const vector<string> &l, const vector<string> &r);

    /**
     * Similar to its version with two input parameters.
     */
    static vector<string> mergeToNewVector(const vector<string> &a, const vector<string> &b, const vector<string> &c);

};


#endif //FASTQINDEX_ERRORACCUMULATOR_H
