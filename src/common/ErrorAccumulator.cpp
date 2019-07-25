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

void ErrorAccumulator::always(_cstr s0, _cstr s1, _cstr s2, _cstr s3, _cstr s4, _cstr s5) {
    cerr << ErrorAccumulator::join(s0, s1, s2, s3, s4, s5) << "\n";
}

void ErrorAccumulator::debug(_cstr s0, _cstr s1, _cstr s2, _cstr s3, _cstr s4, _cstr s5) {
    if (verbosityIsSetToDebug()) cerr << ErrorAccumulator::join(s0, s1, s2, s3, s4, s5) << "\n";
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

const void ErrorAccumulator::addErrorMessage(_cstr s0, _cstr s1, _cstr s2, _cstr s3, _cstr s4, _cstr s5) {
    ErrorAccumulator::debug(s0, s1, s2, s3, s4, s5);
    errorMessages.emplace_back(join(s0, s1, s2, s3, s4, s5));
}

string ErrorAccumulator::join(_cstr s0, _cstr s1, _cstr s2, _cstr s3, _cstr s4, _cstr s5) {
    ostringstream stream(s0);
    stream << s0 << s1 << s2 << s3 << s4 << s5;
    return stream.str();
}


vector<string> ErrorAccumulator::mergeToNewVector(const vector<string> &l, const vector<string> &r) {
    std::vector<string> merged;
    merged.reserve(l.size() + r.size());
    merged.insert(merged.end(), l.begin(), l.end());
    merged.insert(merged.end(), r.begin(), r.end());
    return merged;
}

vector<string>
ErrorAccumulator::mergeToNewVector(const vector<string> &a, const vector<string> &b, const vector<string> &c) {
    std::vector<string> merged;
    merged.reserve(a.size() + b.size() + c.size());
    merged.insert(merged.end(), a.begin(), a.end());
    merged.insert(merged.end(), b.begin(), b.end());
    merged.insert(merged.end(), c.begin(), c.end());
    return merged;
}