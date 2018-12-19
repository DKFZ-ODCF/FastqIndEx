/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_ERRORMESSAGES_H
#define FASTQINDEX_ERRORMESSAGES_H

#include <string>

const std::string ERR_MESSAGE_FASTQ_INVALID("The fastq file does not exist or cannot be read.");
const std::string ERR_MESSAGE_INDEX_INVALID("The index file already exists and is either not a file or cannot be written.");

#endif //FASTQINDEX_ERRORMESSAGES_H
