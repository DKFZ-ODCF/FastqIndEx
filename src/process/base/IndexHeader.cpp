/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexHeader.h"

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