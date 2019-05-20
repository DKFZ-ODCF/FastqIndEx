//
// Created by heinold on 07.02.19.
//

#ifndef FASTQINDEX_ERRORACCUMULATOR_H
#define FASTQINDEX_ERRORACCUMULATOR_H

#include <string>
#include <vector>
#include <iostream>

using std::vector;
using std::string;

/**
 * The class is used to provide basic error storage methods.
 */
class ErrorAccumulator {

private:

    /**
     * Keep a list of errors which came up during checkPremises()
     */
    vector<string> errorMessages;

    static int verbosity;

public:

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

    static void debug(string msg);

    static void info(string msg);

    static void warning(string msg);

    static void severe(string msg);

    virtual vector<string> getErrorMessages();

    const void addErrorMessage(const string &message);

    /**
     * This method can be used, if two vectors should be merged. Note, that we always copy the content of the two source
     * vectors into the new vector. However, as this is method is mostly intended to be used with the ErrorAccumulator
     * messages and these are only requested and, when errors came up, we'll accept the price of copy.
     * @param l The left vector
     * @param r The right vector
     * @return A new vector<string> with both vectors merged. The messages of l will be placed before the messages of r.
     */
    static vector<string> mergeToNewVector(const vector<string> &l, const vector<string> &r);

};


#endif //FASTQINDEX_ERRORACCUMULATOR_H
