/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_COMMONSTRUCTS_H
#define FASTQINDEX_COMMONSTRUCTS_H

#include <cstring>
#include <zconf.h>

/**
 * Used to identify a file as a file created by this binary.
 */
extern const uint MAGIC_NUMBER;

/**
 * The header for an gz index file
 * The size of the header struct is 512Byte including 4Byte overhead by padding.
 * See IndexHeaderV1 for more information about padding.
 */
struct IndexHeader {

    /**
     * Stores the version of the IndexerWriter component.
     */
    uint indexWriterVersion;

    /**
     * Keep track of the size of the stored index entries.
     */
    uint sizeOfIndexEntry;

    /**
     * A magic number to identify index files which were created
     * with this software.
     */
    uint magicNumber;

    /**
     * Reserved space for information which might be added in
     * the future.
     */
    ulong reserved[62]{0};

    explicit IndexHeader(uint binaryVersion, uint sizeOfIndexEntry) {
        magicNumber = MAGIC_NUMBER;
        this->indexWriterVersion = binaryVersion;
        this->sizeOfIndexEntry = sizeOfIndexEntry;
    }

    bool operator==(const IndexHeader &rhs) const;

    bool operator!=(const IndexHeader &rhs) const {
        return !(rhs == *this);
    }
};

struct VirtualIndexEntry {
    bool operator==(const VirtualIndexEntry &rhs) const { return true; };
};

/**
 * Represents an entry in a gz index file
 * The order of the entries is set so, that we have a reduced padding of only 2 Bytes.
 * Please take a look here for some explanations:
 *   https://stackoverflow.com/questions/119123/why-isnt-sizeof-for-a-struct-equal-to-the-sum-of-sizeof-of-each-member
 * The size of this struct is 24 Bytes.
 */
struct IndexEntryV1 : public VirtualIndexEntry {

    /**
     * Number of the entry, not of the line in the FASTQ
     * Each entry marks an entry point into a gz file
     */
    unsigned int entryNumber{0};        // Will align with bits and entryStartsWith to 8Byte

    unsigned char bits{0};

    bool entryStartsWithLine{false};

    unsigned long offset{0};

    unsigned long startingLineInEntry{0};

    IndexEntryV1(unsigned int entryNumber,
                 unsigned long offset,
                 unsigned char bits,
                 unsigned long startingLineInEntry,
                 bool entryStartsWithLine) :
            entryNumber(entryNumber),
            bits(bits),
            entryStartsWithLine(entryStartsWithLine),
            offset(offset),
            startingLineInEntry(startingLineInEntry) {}

    IndexEntryV1() = default;

    bool operator==(const IndexEntryV1 &rhs) const;

    bool operator!=(const IndexEntryV1 &rhs) const {
        return !(rhs == *this);
    }
};

#endif //FASTQINDEX_COMMONSTRUCTS_H
