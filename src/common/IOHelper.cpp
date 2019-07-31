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

recursive_mutex IOHelper::iohelper_mtx;

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

tuple<bool, path> IOHelper::createTempDir(const string &prefix) {
    lock_guard<recursive_mutex> lockGuard(IOHelper::iohelper_mtx);
    auto tempDir = temp_directory_path();
    string testDir = tempDir.string() + "/" + prefix + "_XXXXXXXXXXXXXX";
    char *buf = new char[testDir.size() + 1]{0};
    testDir.copy(buf, testDir.size(), 0);
    char *result = mkdtemp(buf);
    path _path(result); // Create result before deleting buf
    delete[] buf;
    return {result != nullptr, _path};
}

tuple<bool, path> IOHelper::createTempFile(const string &prefix) {
    lock_guard<recursive_mutex> lockGuard(IOHelper::iohelper_mtx);
    auto _tempDir = temp_directory_path();
    string _tempS3File = _tempDir.string() + "/" + prefix + "_XXXXXXXXXXXXXX";
    char *buf = new char[_tempS3File.size() + 1]{0};
    _tempS3File.copy(buf, _tempS3File.size(), 0);
    auto result = mkstemp(buf);
    close(result);
    path tmp = path(string(buf));
    delete[] buf;
    return {result != -1, tmp};
}

tuple<bool, path> IOHelper::createFifo(const string &prefix) {
    lock_guard<recursive_mutex> lockGuard(IOHelper::iohelper_mtx);
    auto[success, fifoPath] = createTempFile(prefix);
    remove(fifoPath);
    mkfifo(fifoPath.string().c_str(), 0666);
    return {success, fifoPath};
}

path IOHelper::fullPath(const path &file) {
    char buf[32768]{0};
    realpath(file.string().c_str(), buf);
    return path(string(buf));
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
