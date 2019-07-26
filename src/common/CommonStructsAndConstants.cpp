/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "CommonStructsAndConstants.h"

using namespace std;

const u_char MAGIC_NUMBER_RAW[4] = {1, 2, 3, 4};

/**
 * Magic number to identify FastqIndEx index files.
 * Basically (((uint) 4) << 24) + (((uint) 3) << 16) + (((uint) 2) << 8) + 1;
 */
const uint MAGIC_NUMBER = *((uint *) MAGIC_NUMBER_RAW);

/**
 * Created by CLion
 */
bool IndexHeader::operator==(const IndexHeader &rhs) const {
    // We do not compare the reserved space
    return indexWriterVersion == rhs.indexWriterVersion &&
           blockInterval == rhs.blockInterval &&
           sizeOfIndexEntry == rhs.sizeOfIndexEntry &&
           magicNumber == rhs.magicNumber &&
           dictionariesAreCompressed == rhs.dictionariesAreCompressed &&
           linesInIndexedFile == rhs.linesInIndexedFile;
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
        blockOffsetInRawFile(offsetInRawFile),
        startingLineInEntry(startingLineInEntry),
        offsetOfFirstValidLine(offsetOfFirstValidLine),
        bits(bits) {}

bool IndexEntry::operator==(const IndexEntry &rhs) const {
    return bits == rhs.bits &&
           id == rhs.id &&
           offsetOfFirstValidLine == rhs.offsetOfFirstValidLine &&
           blockOffsetInRawFile == rhs.blockOffsetInRawFile &&
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

shared_ptr<IndexEntry> IndexEntryV1::toIndexEntry() {
    auto indexLine = make_shared<IndexEntry>(
            this->blockID,
            this->bits,
            this->offsetOfFirstValidLine,
            this->blockOffsetInRawFile,
            this->startingLineInEntry
    );
    memcpy(indexLine->window, this->dictionary, sizeof(this->dictionary));
    indexLine->compressedDictionarySize = this->compressedDictionarySize;
    return indexLine;
}
