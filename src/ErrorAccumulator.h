//
// Created by heinold on 07.02.19.
//

#ifndef FASTQINDEX_ERRORACCUMULATOR_H
#define FASTQINDEX_ERRORACCUMULATOR_H

#include <string>
#include <vector>

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

public:

    virtual vector<string> getErrorMessages() { return errorMessages; }

    const void addErrorMessage(const string &message) {
        errorMessages.emplace_back(message);
    }

    /**
     * This method can be used, if two vectors should be merged. Note, that we always copy the content of the two source
     * vectors into the new vector. However, as this is method is mostly intended to be used with the ErrorAccumulator
     * messages and these are only requested and, when errors came up, we'll accept the price of copy.
     * @param l The left vector
     * @param r The right vector
     * @return A new vector<string> with both vectors merged. The messages of l will be placed before the messages of r.
     */
    static vector<string> mergeToNewVector(const vector<string> &l, const vector<string> &r) {
        std::vector<string> merged;
        merged.reserve(l.size() + r.size());
        merged.insert(merged.end(), l.begin(), l.end());
        merged.insert(merged.end(), r.begin(), r.end());
        return merged;
    };

};


#endif //FASTQINDEX_ERRORACCUMULATOR_H
