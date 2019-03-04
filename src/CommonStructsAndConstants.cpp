/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "CommonStructsAndConstants.h"

const uint MAGIC_NUMBER = (((uint) 4) << 24) + (((uint) 3) << 16) + (((uint) 2) << 8) + 1;

const uint CHUNK_SIZE = 16 * 1024;

const uint WINDOW_SIZE = 32 * 1024;

const uint CLEAN_WINDOW_SIZE = WINDOW_SIZE + 1;

/**
 * Created by CLion
 */
bool IndexHeader::operator==(const IndexHeader &rhs) const {
    // We do not compare the reserved space
    return indexWriterVersion == rhs.indexWriterVersion &&
           blockInterval == rhs.blockInterval &&
           sizeOfIndexEntry == rhs.sizeOfIndexEntry &&
           magicNumber == rhs.magicNumber;
}

/**
 * Mainly for test.
 * @return
 */
IndexHeader::operator bool() const {
    return magicNumber == MAGIC_NUMBER &&
           blockInterval > 0 &&
           (indexWriterVersion == 1 && sizeOfIndexEntry == sizeof(IndexEntryV1));
}

/**
 * Created by CLion
 */
bool IndexEntryV1::operator==(const IndexEntryV1 &rhs) const {
//    return static_cast<const VirtualIndexEntry &>(*this) == static_cast<const VirtualIndexEntry &>(rhs) &&
    return bits == rhs.bits &&
           blockID == rhs.blockID &&
           offsetOfFirstValidLine == rhs.offsetOfFirstValidLine &&
           blockOffsetInRawFile == rhs.blockOffsetInRawFile &&
           startingLineInEntry == rhs.startingLineInEntry;
}

IndexLine::IndexLine(ulong id,
                     unsigned char bits,
                     ushort offsetOfFirstValidLine,
                     ulong offsetInRawFile,
                     ulong startingLineInEntry) :
        id(id),
        offsetInRawFile(offsetInRawFile),
        startingLineInEntry(startingLineInEntry),
        offsetOfFirstValidLine(offsetOfFirstValidLine),
        bits(bits) {}
