/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "ActualRunner.h"
#include "../src/ErrorMessages.h"
#include <boost/filesystem.hpp>
#include <error.h>

using namespace boost::filesystem;

ActualRunner::ActualRunner(path fastqfile, path indexfile) {
    this->fastqFile = fastqfile;
    this->indexFile = indexfile;
}

bool ActualRunner::checkPremises() {

    // TODO Will need to be extended for pipe i/o

    // Fastq needs to be an ((existing file OR symlink with a file) AND readable)
    path fastqFile = path(this->fastqFile);
    path indexFile = path(this->indexFile);

    // NOTE I wanted to upgrade Boost from 1.54 to 1.69/1.64 because of the child process features.
    // However, as soon as I switched, nearly all tests failed and e.g. here the following happened:
    // - After the call of the first exists / is_symlink, the passed variable got corrupted!
    // - If we took the original class field, the whole class instance was corrupted!
    // - I am not sure yet, how we can report this.
    bool fastqIsValid = false;
    if (!exists(fastqFile)) {
        addErrorMessage("The selected FASTQ file does not exist.");
    } else {

        if (is_symlink(symlink_status(fastqFile)))
            fastqFile = read_symlink(fastqFile);

        fastqIsValid = is_regular_file(fastqFile);

        if (!fastqIsValid)
            addErrorMessage(ERR_MESSAGE_FASTQ_INVALID);
    }

    // Index files are automatically overwrite but need to have write access!
    bool indexIsValid = true;
    // It is totally ok, if the index does not exist. We'll create it then.
    if (exists(indexFile)) {
        if (is_symlink(indexFile))
            indexFile = read_symlink(indexFile);

        if (!is_regular(indexFile)) {
            indexIsValid = false;
            addErrorMessage(ERR_MESSAGE_INDEX_INVALID);
        }
    }

    return fastqIsValid && indexIsValid;
}
