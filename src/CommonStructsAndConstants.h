/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_COMMONSTRUCTS_H
#define FASTQINDEX_COMMONSTRUCTS_H

#include <cstring>
#include <string>
#include <zconf.h>

// Forward declaration of IndexEntryV[n]
struct IndexEntryV1;

/**
 * Used to identify a file as a file created by this binary.
 */
extern const uint MAGIC_NUMBER;

/**
 * Chunk size for raw data from compressed file
 */
extern const uint CHUNK_SIZE;

/**
 * Size of buffer for decompressed data
 */
extern const uint WINDOW_SIZE;

/**
 * As we mix strings, stringstreams and cstrings, a buffer with this size is used to ensure, that the decompressed data
 * copied to the cleansed buffer is zero terminated.
 */
extern const uint CLEAN_WINDOW_SIZE;

/**
 * The header for an gz index file
 * The size of the header struct is 512Byte including 4Byte overhead by padding.
 * See IndexHeaderV1 for more information about padding.
 */
struct IndexHeader {

    /**
     * Stores the version of the IndexerWriter component.
     */
    uint indexWriterVersion{0};

    /**
     * Keep track of the size of the stored index entries.
     */
    uint sizeOfIndexEntry{0};

    /**
     * A magic number to identify index files which were created
     * with this software.
     */
    uint magicNumber = MAGIC_NUMBER;

    /**
     * The interval of blocks between index entries.
     */
    uint blockInterval{0};

    /**
     * Reserved space for information which might be added in
     * the future.
     */
    ulong reserved[62]{0};

    explicit IndexHeader(uint binaryVersion, uint sizeOfIndexEntry, uint blockInterval) {
        this->indexWriterVersion = binaryVersion;
        this->sizeOfIndexEntry = sizeOfIndexEntry;
        this->blockInterval = blockInterval;
    }

    IndexHeader() = default;

    bool operator==(const IndexHeader &rhs) const;

    bool operator!=(const IndexHeader &rhs) const {
        return !(rhs == *this);
    }

    explicit operator bool() const;

    bool operator!() { return !this; }
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
     * The identifier of the raw compressed block for which this entry is.
     */
    ulong blockID{0};

    /**
     * Does this block / entry start with a fresh line? If so, offset is zero, otherwise the position of the first line.
     */
    ushort offsetOfFirstValidLine{0};

    /**
     * Block offset in raw gz file. Note, that this is not the absolute offset but a relative offset to the last entry.
     * That said, you need to read in the whole index file, before you can effectively use it.
     */
    ulong blockOffsetInRawFile{0};

    /**
     * The id of the first valid line in the block / entry. Note, that this value is relative to the last entry. That
     * said, you need to read in the whole index file, before you can effectively use it.
     */
    ulong startingLineInEntry{0};

    unsigned char bits{0};
//
//    /**
//     * Size of the compressed dictionary data in Byte. See below.
//     */
//    ulong compressedDictionarySize{0};

    /**
     * The dictionary is a chunk of uncompressed data from the data block before the block to which this entry points.
     * Before we decompress in raw mode, we need to initialize zlib with this dictionary, otherwise the decompression
     * either fails or, what's worse, produces garbage. In comparison to the zran.c example by Marc Adler, we don't copy
     * around decompressed data but use the more speaking inflateGetDictionary method.
     *
     * As in zindex by Matt Godbolt, we will compress the window before storing it in a file. This makes storage a bit
     * more complicated, but we save around 60% of storage space. This is for a future step.
     */
    Bytef dictionary[32768]{0};

//    std::string firstLine;

    IndexEntryV1(unsigned char bits,
                 ulong blockID,
                 ushort offsetOfFirstValidLine,
                 ulong offsetInRawFile,
                 ulong startingLineInEntry) :
            bits(bits),
            blockID(blockID),
            offsetOfFirstValidLine(offsetOfFirstValidLine),
            blockOffsetInRawFile(offsetInRawFile),
            startingLineInEntry(startingLineInEntry) {}

    IndexEntryV1() = default;

    bool operator==(const IndexEntryV1 &rhs) const;

    bool operator!=(const IndexEntryV1 &rhs) const {
        return !(rhs == *this);
    }
};

/**
 * A (decompressed / processed) representation of of an IndexEntryV[n] with total value instead of relative values.
 */
struct IndexLine {
    ulong id{0};
    ulong offsetInRawFile{0};
    ulong startingLineInEntry{0};
    ushort offsetOfFirstValidLine{0};
    ulong compressedDictionarySize{0};
    int bits{0};
    Bytef window[32768]{0};

    IndexLine(ulong id,
              unsigned char bits,
              ushort offsetOfFirstValidLine,
              ulong offsetInRawFile,
              ulong startingLineInEntry);

    IndexLine() = default;
};

#endif //FASTQINDEX_COMMONSTRUCTS_H
