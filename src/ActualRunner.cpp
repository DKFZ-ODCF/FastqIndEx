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

    // TODO Will need to be extended for pipe i/o

    // Fastq needs to be an ((existing file OR symlink with a file) AND readable)

    if (is_symlink(fastqFile))
        fastqFile = read_symlink(fastqFile);

    bool fastqIsValid = is_regular_file(fastqFile);

    if (!fastqIsValid)
        addErrorMessage(ERR_MESSAGE_FASTQ_INVALID);

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
