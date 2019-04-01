/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <iostream>
#include "ActualRunner.h"
#include "../src/ErrorMessages.h"
#include <error.h>
#include <experimental/filesystem>

using experimental::filesystem::path;

ActualRunner::ActualRunner(const path &fastqfile, const path &indexfile) {
    this->fastqFile = fastqfile;
    this->indexFile = indexfile;
}

bool ActualRunner::checkPremises() {

    // TODO Will need to be extended for pipe i/o

    // Fastq needs to be an ((existing file OR symlink with a file) AND readable)
    path fastqFile = path(this->fastqFile);
    path indexFile = path(this->indexFile);

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

        if (!is_regular_file(indexFile)) {
            indexIsValid = false;
            addErrorMessage(ERR_MESSAGE_INDEX_INVALID);
        }
    }

    return fastqIsValid && indexIsValid;
}
