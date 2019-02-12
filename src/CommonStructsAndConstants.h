/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
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
 * The size of this struct is 8 Bytes.
 */
struct IndexEntryV1 : public VirtualIndexEntry {

    /**
     * Does this block / entry start with a fresh line? If so, offset is zero, otherwise the position of the first line.
     */
    ushort offsetOfFirstValidLine{0};

    /**
     * Block offset in raw gz file. Note, that this is not the absolute offset but a relative offset to the last entry.
     * That said, you need to read in the whole index file, before you can effectively use it.
     */
    ushort relativeBlockOffsetInRawFile{0};

    /**
     * The id of the first valid line in the block / entry. Note, that this value is relative to the last entry. That
     * said, you need to read in the whole index file, before you can effectively use it.
     */
    ushort startingLineInEntry{0};

    unsigned char bits{0};

    IndexEntryV1(unsigned char bits,
                 ushort offsetOfFirstValidLine,
                 ushort offsetInRawFile,
                 ushort startingLineInEntry) :
            bits(bits),
            offsetOfFirstValidLine(offsetOfFirstValidLine),
            relativeBlockOffsetInRawFile(offsetInRawFile),
            startingLineInEntry(startingLineInEntry) {}

    IndexEntryV1() = default;

    bool operator==(const IndexEntryV1 &rhs) const;

    bool operator!=(const IndexEntryV1 &rhs) const {
        return !(rhs == *this);
    }
};

#endif //FASTQINDEX_COMMONSTRUCTS_H
