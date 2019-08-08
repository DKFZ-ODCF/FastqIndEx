/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/ErrorMessages.h"
#include "process/io/StreamSource.h"
#include "runners/ActualRunner.h"
#include <experimental/filesystem>

using experimental::filesystem::path;

ActualRunner::ActualRunner(const shared_ptr<Source> &fastqfile) {
    this->fastqFile = fastqfile;
}

bool ActualRunner::fulfillsPremises() {

    // Fastq needs to be a (pipe AND piping allowed) or an ((existing file OR symlink with a file) AND readable)
    bool fastqIsValid = false;

    if (fastqFile->isFile()) {
        if (!fastqFile->exists()) {
            addErrorMessage("The selected FASTQ file '" + fastqFile->toString() + "' does not exist.");
        } else {
            fastqIsValid = true;
        }
    } else {
        if (!allowsReadFromStreamedSource()) {
            addErrorMessage("You are not allowed to use piped input for the current mode.");
        } else {
            fastqIsValid = true;
        }
    }

    return fastqIsValid;
}

bool IndexReadingRunner::fulfillsPremises() {
    bool fastqIsValid = ActualRunner::fulfillsPremises();
    bool indexIsValid = indexFile->fulfillsPremises();

    return fastqIsValid && indexIsValid;
}

vector<string> IndexReadingRunner::getErrorMessages() {
    if (fastqFile.get()) {
        auto a = ErrorAccumulator::getErrorMessages();
        auto b = fastqFile->getErrorMessages();
        auto c = indexFile->getErrorMessages();
        return mergeToNewVector(a, b, c);
    } else {
        auto a = ErrorAccumulator::getErrorMessages();
        auto c = indexFile->getErrorMessages();
        return mergeToNewVector(a, c);
    }
}

bool IndexWritingRunner::fulfillsPremises() {

    bool fastqIsValid = ActualRunner::fulfillsPremises();
    bool indexIsValid = indexFile->fulfillsPremises();
//    // Index files are automatically overwritten but need to have write access!
//    bool indexIsValid = true;
//    // It is totally ok, if the index does not exist. We'll create it then.
//    if (exists(inputIndexFile)) {
//        if (is_symlink(inputIndexFile))
//            inputIndexFile = read_symlink(inputIndexFile);
//
//        if (!is_regular_file(inputIndexFile)) {
//            indexIsValid = false;
//            addErrorMessage(ERR_MESSAGE_INDEX_INVALID);
//        }
//    }

    return fastqIsValid && indexIsValid;
}

vector<string> IndexWritingRunner::getErrorMessages() {
    auto a = ErrorAccumulator::getErrorMessages();
    auto b = fastqFile->getErrorMessages();
    auto c = indexFile->getErrorMessages();
    return mergeToNewVector(a, b, c);
}

