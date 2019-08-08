/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXHEADER_H
#define FASTQINDEX_INDEXHEADER_H

#include "CommonStructsAndConstants.h"
#include "IndexEntry.h"
#include "IndexEntryV1.h"

/**
 * The header for an gz index file
 * The size of the header struct is 512Byte including 4Byte overhead by padding.
 * See IndexHeaderV1 for more information about padding.
 */
struct IndexHeader {

    /**
     * Stores the version of the IndexerWriter component.
     */
    u_int32_t indexWriterVersion{0};

    /**
     * Keep track of the size of the stored index entries.
     */
    u_int32_t sizeOfIndexEntry{0};

    /**
     * A magic number to identify index files which were created
     * with this software.
     */
    u_int32_t magicNumber = MAGIC_NUMBER;

    /**
     * The interval of blocks between index entries.
     */
    u_int32_t blockInterval{0};

    /**
     * This value is not available when a header is created in write mode. It will be written at the end of indexing
     * process. However, the indexer and the extractor will work without it.
     */
    int64_t numberOfEntries{0};

    /**
     * Stores the amount of lines in the indexed file. This value is also written after the indexing is done.
     * However, the indexer and the extractor will work without it.
     */
    int64_t linesInIndexedFile{0};

    /**
     * Tell the IndexReader, if the entries are compressed. (1-byte padded to 8 bytes!) The value looks weird without
     * the followup value when you look it up in written fqi files. C++ does not do a clean write in this case.
     */
    bool dictionariesAreCompressed{false};
    Bytef placeholder[7]{0};

    /**
     * Reserved space for information which might be added in
     * the future.
     */
    int64_t reserved[59]{0};

    explicit IndexHeader(u_int32_t binaryVersion, u_int32_t sizeOfIndexEntry, u_int32_t blockInterval, bool dictionariesAreCompressed) {
        this->indexWriterVersion = binaryVersion;
        this->sizeOfIndexEntry = sizeOfIndexEntry;
        this->blockInterval = blockInterval;
        this->dictionariesAreCompressed = dictionariesAreCompressed;
    }

    IndexHeader() = default;

    bool operator==(const IndexHeader &rhs) const;

    bool operator!=(const IndexHeader &rhs) const {
        return !(rhs == *this);
    }

    explicit operator bool() const;

    bool operator!() { return !this; }
};


#endif //FASTQINDEX_INDEXHEADER_H
