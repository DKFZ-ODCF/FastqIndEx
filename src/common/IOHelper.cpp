/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IOHelper.h"
#include <fstream>
#include <pwd.h>
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

path IOHelper::getUserHomeDirectory() {
    // The manpage of getuid states:
    // The getuid() function shall always be successful and no return value is reserved to indicate the error.
    // Therefore, we have a valid uid and getpwuid should not fail as well. No further checks applied here.
    struct passwd *pw = getpwuid(getuid());
    if (pw == nullptr) {
        auto s = stringstream();
        s << "FastqIndEx could not retrieve the user directory of the current user.";
        report(s, nullptr);
    }
    // According to the manpages, the struct is not to be free'd!
    return path(string(pw->pw_dir));
}

bool IOHelper::checkFileReadability(const path &file, const string &fileType, ErrorAccumulator *errorAccumulator) {
    bool _isValid = exists(file);
    if (!_isValid) {
        std::stringstream sstream;
        sstream << "The " << fileType << " file '" << file.string() << "' could not be found or is inaccessible.";
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

tuple<bool, path> IOHelper::createTempFifo(const string &prefix) {
    lock_guard<recursive_mutex> lockGuard(IOHelper::iohelper_mtx);
    auto[success, fifoPath] = createTempFile(prefix);
    remove(fifoPath);
    // The mkfifo manpage states, that the requested mode is &'ded with the users umask.
    mkfifo(fifoPath.string().c_str(), 0600);
    return {success, fifoPath};
}

/**
 * Careful, this method does not necessarily work! Tried it with a binary file path and it got me some path in my home
 * directory. The binary was somewhere else!.
 */
path IOHelper::fullPath(const path &file) {
    char buf[32768]{0};
    //    readlink(file.string().c_str(), buf, 32768); // Not working! Returns \0
    realpath(file.string().c_str(), buf);
    return path(string(buf));
}

/**
 * Retrieve the absolute path of the current executable. This will only work with operating systems which use a /proc
 * or /user folder to hold process information.
 */
path IOHelper::getApplicationPath() {
    char buf[32768]{0};
    if (exists("/proc"))
        readlink("/proc/self/exe", buf, 32768);
    else if (exists("/user"))
        readlink("/user/self/exe", buf, 32768);
    else
        ErrorAccumulator::severe(string("BUG: Could not the folders '/proc' or '/user'. ") +
                                 "Are you running FastqIndEx on a compatible system?");
    return path(string(buf));
}

shared_ptr<map<string, string>> IOHelper::loadIniFile(const path &file, const string &section) {
    auto resultMap = make_shared<map<string, string>>();
    CSimpleIniA configuration;
    configuration.SetUnicode(true);
    configuration.LoadFile(file.string().c_str());
    CSimpleIniA::TNamesDepend keys;
    configuration.GetAllKeys(section.c_str(), keys);
    for (const auto &key : keys) {
        auto val = configuration.GetValue(section.c_str(), key.pItem);
        (*resultMap)[string(key.pItem)] = string(val);
    }
    return resultMap;
}
