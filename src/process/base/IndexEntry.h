/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXENTRY_H
#define FASTQINDEX_INDEXENTRY_H

#include "common/CommonStructsAndConstants.h"

struct IndexEntry;

/**
 * Shortcut for the shared pointer version of IndexEntry
 */
typedef std::shared_ptr<IndexEntry> IndexEntry_S;

/**
 * A (decompressed / processed) representation of an IndexEntryV[n].
 *
 * For an explanation, refer to IndexEntryV1
 */
struct IndexEntry {
    u_int64_t id{0};
    u_int64_t blockOffsetInRawFile{0};
    u_int64_t startingLineInEntry{0};
    u_int64_t compressedDictionarySize{0};
    u_int32_t bits{0};
    u_int16_t offsetToNextLineStart{0};

    Bytef window[WINDOW_SIZE]{0};

    IndexEntry(u_int64_t id,
               u_int32_t bits,
               u_int16_t offsetOfFirstValidLine,
               u_int64_t offsetInRawFile,
               u_int64_t startingLineInEntry);

    explicit IndexEntry(u_int64_t id) {
        this->id = id;
    }

    IndexEntry() = default;

    bool operator==(const IndexEntry &rhs) const;

    bool operator!=(const IndexEntry &rhs) const { return !(rhs == *this); }

};


#endif //FASTQINDEX_INDEXENTRY_H
