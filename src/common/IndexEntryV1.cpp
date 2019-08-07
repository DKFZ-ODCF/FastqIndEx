/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexEntryV1.h"

using namespace std;

/**
 * Created by CLion
 */
bool IndexEntryV1::operator==(const IndexEntryV1 &rhs) const {
    return bits == rhs.bits &&
           blockID == rhs.blockID &&
           offsetToNextLineStart == rhs.offsetToNextLineStart &&
           blockOffsetInRawFile == rhs.blockOffsetInRawFile &&
           startingLineInEntry == rhs.startingLineInEntry;
}

shared_ptr<IndexEntry> IndexEntryV1::toIndexEntry() {
    auto indexLine = make_shared<IndexEntry>(
            this->blockID,
            this->bits,
            this->offsetToNextLineStart,
            this->blockOffsetInRawFile,
            this->startingLineInEntry
    );
    memcpy(indexLine->window, this->dictionary, sizeof(this->dictionary));
    indexLine->compressedDictionarySize = this->compressedDictionarySize;
    return indexLine;
}