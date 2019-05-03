/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "ActualRunner.h"
#include "../src/ErrorMessages.h"
#include "StreamInputSource.h"
#include "PathInputSource.h"
#include <error.h>
#include <experimental/filesystem>

using experimental::filesystem::path;

ActualRunner::ActualRunner(const path &fastqfile, const path &indexfile) {
    this->fastqFile = make_shared<PathInputSource>(fastqfile);
    this->indexFile = indexfile;
    debug(string(
            "Created runner with fastq file: \"" + fastqfile.string() + "\" and index \"" + indexfile.string() + "\""));
}

ActualRunner::ActualRunner(istream *fastqStream, const path &indexfile) {
    this->fastqFile = make_shared<StreamInputSource>(fastqStream);
    this->indexFile = indexfile;
    debug(string("Created runner with input stream and index \"" + indexfile.string() + "\""));
}

ActualRunner::ActualRunner(const shared_ptr<InputSource> &fastqfile, const path &indexfile) {
    this->fastqFile = fastqfile;
    this->indexFile = indexfile;
    if (fastqfile->isStreamSource())
        debug(string("Created runner with input stream and index \"" + indexfile.string() + "\""));
    else
        debug(string("Created runner with fastq file: \"" +
                     dynamic_pointer_cast<PathInputSource>(fastqfile)->getPath().string() +
                     "\" and index \"" + indexfile.string() + "\""));
}

bool ActualRunner::checkPremises() {

    // Fastq needs to be a (pipe AND piping allowed) or an ((existing file OR symlink with a file) AND readable)
    bool fastqIsValid = false;

    if (fastqFile->isStreamSource()) {
        if (!allowsReadFromStreamedSource()) {
            addErrorMessage("You are not allowed to use piped input for the current mode.");
        } else {
            fastqIsValid = true;
        }
    } else {
        auto fq = dynamic_pointer_cast<PathInputSource>(fastqFile);
        if (!fastqFile->exists()) {
            addErrorMessage("The selected FASTQ file '" + fq->absolutePath() + "' does not exist.");
        } else {
            if (!fq->isRegularFile())
                addErrorMessage(ERR_MESSAGE_FASTQ_INVALID);
            else
                fastqIsValid = true;
        }
    }

    // Index files are automatically overwritten but need to have write access!
    bool indexIsValid = true;
    // It is totally ok, if the index does not exist. We'll create it then.
    if (exists(indexFile)) {
        if (is_symlink(indexFile))
            indexFile = read_symlink(indexFile);

        if (!is_regular_file(indexFile)) {
            indexIsValid = false;
            addErrorMessage(ERR_MESSAGE_INDEX_INVALID);
        }
    }

    return fastqIsValid && indexIsValid;
}
