/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
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
    // Fastq needs to be an ((existing file OR symlink with a file) AND readable)

    if (is_symlink(fastqFile))
        fastqFile = read_symlink(fastqFile);

    bool fastqIsValid = is_regular_file(fastqFile);

    if (!fastqIsValid)
        errorMessages.push_back(ERR_MESSAGE_FASTQ_INVALID);

    // Index files are automatically override but need to have write access!
    bool indexIsValid = true;
    if (exists(indexFile)) {
        if (is_symlink(indexFile))
            indexFile = read_symlink(indexFile);

        if (!is_regular(indexFile)) {
            indexIsValid = false;
            errorMessages.push_back(ERR_MESSAGE_INDEX_INVALID);
        }

    } // It is totally ok, if the index does not exists. We'll create it then.

    return fastqIsValid && indexIsValid;
}
