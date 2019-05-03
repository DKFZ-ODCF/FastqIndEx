/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "ErrorAccumulator.h"
#include <iostream>

using namespace std;

int ErrorAccumulator::verbosity = 0;

void ErrorAccumulator::setVerbosity(int verbosity) {
    if (verbosity > 0 && verbosity <= 3) {
        if (verbosity > 0)
            cerr << "Set verbosity to level " << verbosity << "\n";
        ErrorAccumulator::verbosity = verbosity;
    }
}

void ErrorAccumulator::debug(string msg) {
    if (verbosity >= 3) cerr << msg << "\n";
}

void ErrorAccumulator::info(string msg) {
    if (verbosity >= 2) cerr << msg << "\n";
}

void ErrorAccumulator::warning(string msg) {
    if (verbosity >= 1) cerr << msg << "\n";
}

void ErrorAccumulator::severe(string msg) {
    if (verbosity >= 0) cerr << msg << "\n";
}

vector<string> ErrorAccumulator::getErrorMessages() { return errorMessages; }

const void ErrorAccumulator::addErrorMessage(const string &message) {
    ErrorAccumulator::debug(message);
    errorMessages.emplace_back(message);
}

vector<string> ErrorAccumulator::mergeToNewVector(const vector<string> &l, const vector<string> &r) {
    std::vector<string> merged;
    merged.reserve(l.size() + r.size());
    merged.insert(merged.end(), l.begin(), l.end());
    merged.insert(merged.end(), r.begin(), r.end());
    return merged;
}
