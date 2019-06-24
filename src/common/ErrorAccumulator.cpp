/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "ErrorAccumulator.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int ErrorAccumulator::verbosity = 0;

void ErrorAccumulator::setVerbosity(int verbosity) {
    if (verbosity > 0 && verbosity <= 3) {
        if (verbosity > 0)
            cerr << "Set verbosity to level " << verbosity << "\n";
        ErrorAccumulator::verbosity = verbosity;
    }
}

bool ErrorAccumulator::verbosityIsSetToDebug() { return verbosity >= 3; }

void ErrorAccumulator::debug(const string &msg) {
    if (verbosityIsSetToDebug()) cerr << msg << "\n";
}

void ErrorAccumulator::info(const string &msg) {
    if (verbosity >= 2) cerr << msg << "\n";
}

void ErrorAccumulator::warning(const string &msg) {
    if (verbosity >= 1) cerr << msg << "\n";
}

void ErrorAccumulator::severe(const string &msg) {
    if (verbosity >= 0) cerr << msg << "\n";
}

vector<string> ErrorAccumulator::getErrorMessages() { return errorMessages; }

const void ErrorAccumulator::addErrorMessage(const string &part0, const string &part1, const string &part2,
                                             const string &part3, const string &part4, const string &part5) {
    ostringstream stream(part0);
    stream << part0 << part1 << part2 << part3 << part4 << part5;

    string msg = stream.str();
    ErrorAccumulator::debug(msg);
    errorMessages.emplace_back(msg);
}

vector<string> ErrorAccumulator::mergeToNewVector(const vector<string> &l, const vector<string> &r) {
    std::vector<string> merged;
    merged.reserve(l.size() + r.size());
    merged.insert(merged.end(), l.begin(), l.end());
    merged.insert(merged.end(), r.begin(), r.end());
    return merged;
}
