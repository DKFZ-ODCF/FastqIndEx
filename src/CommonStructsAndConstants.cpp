/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "CommonStructsAndConstants.h"

const uint MAGIC_NUMBER = (((uint) 4) << 24) + (((uint) 3) << 16) + (((uint) 2) << 8) + 1;

/**
 * Created by CLion
 */
bool IndexHeader::operator==(const IndexHeader &rhs) const {
    // We do not compare the reserved space
    return indexWriterVersion == rhs.indexWriterVersion &&
           sizeOfIndexEntry == rhs.sizeOfIndexEntry &&
           magicNumber == rhs.magicNumber;
}

/**
 * Created by CLion
 */
bool IndexEntryV1::operator==(const IndexEntryV1 &rhs) const {
//    return static_cast<const VirtualIndexEntry &>(*this) == static_cast<const VirtualIndexEntry &>(rhs) &&
    return bits == rhs.bits &&
           offsetOfFirstValidLine == rhs.offsetOfFirstValidLine &&
           relativeBlockOffsetInRawFile == rhs.relativeBlockOffsetInRawFile &&
           startingLineInEntry == rhs.startingLineInEntry;
}
