/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IOHelper.h"
#include <fstream>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <SimpleIni.h>

void IOHelper::report(const stringstream &sstream, ErrorAccumulator *errorAccumulator) {
    if (errorAccumulator)
        errorAccumulator->addErrorMessage(sstream.str());
    else {
        cerr << sstream.str() << "\n";
    }
}

/**
 * Not really testable! I rely on manual testing here.
 * @return The user home directory
 */
path IOHelper::getUserHomeDirectory() {
    struct passwd *pw = getpwuid(getuid());
    // According to the manpages, the struct is not to be free'd!
    return path(string(pw->pw_dir));
}

bool IOHelper::checkFileReadability(const path &file, const string &fileType, ErrorAccumulator *errorAccumulator) {
    bool _isValid = exists(file);
    if (!_isValid) {
        std::stringstream sstream;
        sstream << "The " << fileType << " file '" << file.string() << "' could not be found or is not accessible.";
        report(sstream, errorAccumulator);
    }

    if (_isValid) {
        std::ifstream fileStream(file);
        _isValid &= fileStream.good();
        fileStream.close();

        if (!_isValid) {
            std::stringstream sstream;
            sstream << "The " << fileType << " file '" << file.string() << "' could not be read accessible.";
            report(sstream, errorAccumulator);
        }
    }

    return _isValid;
}

shared_ptr<map<string, string>> IOHelper::loadIniFile(path file, string section) {
    auto resultMap = make_shared<map<string, string>>();
    CSimpleIniA configuration;
    configuration.SetUnicode(true);
    auto result = configuration.LoadFile(file.string().c_str());
    CSimpleIniA::TNamesDepend keys;
    configuration.GetAllKeys(section.c_str(), keys);
    for (auto key : keys) {
        auto val = configuration.GetValue(section.c_str(), key.pItem);
        (*resultMap)[string(key.pItem)] = string(val);
    }
    return resultMap;
}