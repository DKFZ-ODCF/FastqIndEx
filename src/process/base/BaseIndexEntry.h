/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_BASEINDEXENTRY_H
#define FASTQINDEX_BASEINDEXENTRY_H

struct BaseIndexEntry {
    bool operator==(const BaseIndexEntry &rhs) const { return true; };
};

#endif //FASTQINDEX_BASEINDEXENTRY_H
