/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_TESTCONSTANTS_H
#define FASTQINDEX_TESTCONSTANTS_H

extern const char* FASTQ_FILENAME;
extern const char* INDEX_FILENAME;

/**
 * In any case, this value would need to be const. But as we do not have the value when the variable is created, we
 * will just keep it here. At the end, it only stores the path to the test binary and is used for tests only.
 */
extern char TEST_BINARY[16384];

#endif //FASTQINDEX_TESTCONSTANTS_H
