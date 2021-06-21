/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexEntry.h"

IndexEntry::IndexEntry(u_int64_t id,
                       u_int32_t bits,
                       u_int16_t offsetOfFirstValidLine,
                       u_int64_t offsetInRawFile,
                       u_int64_t startingLineInEntry) :
        id(id),
        blockOffsetInRawFile(offsetInRawFile),
        startingLineInEntry(startingLineInEntry),
        offsetToNextLineStart(offsetOfFirstValidLine),
        bits(bits) {}

bool IndexEntry::operator==(const IndexEntry &rhs) const {
    return id == rhs.id &&
           bits == rhs.bits &&
           offsetToNextLineStart == rhs.offsetToNextLineStart &&
           blockOffsetInRawFile == rhs.blockOffsetInRawFile &&
           startingLineInEntry == rhs.startingLineInEntry;
}