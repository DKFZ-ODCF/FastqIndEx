/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_S3LOCKHANDLER_H
#define FASTQINDEX_S3LOCKHANDLER_H

#include "LockHandler.h"

/**
 * S3 does not support a locking model like e.g. flock(), we'll have to work around this issue. However, be aware, that
 * this will not provide the same level of security like flock().
 */
class S3LockHandler : public LockHandler {

};

#endif //FASTQINDEX_S3LOCKHANDLER_H
