/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "CommonStructsAndConstants.h"

const u_char MAGIC_NUMBER_RAW[4] = {1, 2, 3, 4};

/**
 * Magic number to identify FastqIndEx index files.
 * Basically (((uint) 4) << 24) + (((uint) 3) << 16) + (((uint) 2) << 8) + 1;
 */
const uint MAGIC_NUMBER = (*(int*)MAGIC_NUMBER_RAW);

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

IndexEntry::IndexEntry(u_int64_t id,
                       u_int32_t bits,
                       u_int16_t offsetOfFirstValidLine,
                       u_int64_t offsetInRawFile,
                       u_int64_t startingLineInEntry) :
        id(id),
        offsetInRawFile(offsetInRawFile),
        startingLineInEntry(startingLineInEntry),
        offsetOfFirstValidLine(offsetOfFirstValidLine),
        bits(bits) {}

bool IndexEntry::operator==(const IndexEntry &rhs) const {
    return bits == rhs.bits &&
           id == rhs.id &&
           offsetOfFirstValidLine == rhs.offsetOfFirstValidLine &&
           offsetInRawFile == rhs.offsetInRawFile &&
           startingLineInEntry == rhs.startingLineInEntry;
}

/**
 * Created by CLion
 */
bool IndexEntryV1::operator==(const IndexEntryV1 &rhs) const {
    return bits == rhs.bits &&
           blockID == rhs.blockID &&
           offsetOfFirstValidLine == rhs.offsetOfFirstValidLine &&
           blockOffsetInRawFile == rhs.blockOffsetInRawFile &&
           startingLineInEntry == rhs.startingLineInEntry;
}

boost::shared_ptr<IndexEntry> IndexEntryV1::toIndexEntry() {
    auto indexLine = boost::make_shared<IndexEntry>(
            this->blockID,
            this->bits,
            this->offsetOfFirstValidLine,
            this->blockOffsetInRawFile,
            this->startingLineInEntry
    );
    memcpy(indexLine->window, this->dictionary, sizeof(this->dictionary));
    return indexLine;
}
