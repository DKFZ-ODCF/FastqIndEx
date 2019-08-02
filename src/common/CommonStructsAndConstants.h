/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_COMMONSTRUCTS_H
#define FASTQINDEX_COMMONSTRUCTS_H

#include <cstring>
#include <memory>
#include <string>
#include <zconf.h>

using std::shared_ptr;

// Forward declaration of IndexEntryV[n]
struct IndexEntryV1;

/**
 * Used to identify a file as a file created by this binary.
 */
extern const uint MAGIC_NUMBER;

extern const u_int64_t kB;

extern const u_int64_t MB;

extern const u_int64_t GB;

extern const u_int64_t TB;

/**
 * Size of buffer for decompressed data
 * I'd prefer to set the following value as constant ints, but C++ then refuses to initialize arrays with the nice {0}
 * syntax. Though it works, when the constants are in a class.
 */
#define WINDOW_SIZE 32768

/**
 * Chunk size for raw data from compressed file
 */
#define CHUNK_SIZE 16384

/**
 * As we mix strings, stringstreams and cstrings, a buffer with this size is used to ensure, that the decompressed data
 * copied to the cleansed buffer is zero terminated.
 */
#define CLEAN_WINDOW_SIZE (WINDOW_SIZE + 1)

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
    u_int64_t numberOfEntries{0};

    /**
     * Stores the amount of lines in the indexed file. This value is also written after the indexing is done.
     * However, the indexer and the extractor will work without it.
     */
    u_int64_t linesInIndexedFile{0};

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
    u_int64_t reserved[59]{0};

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

/**
 * A (decompressed / processed) representation of an IndexEntryV[n].
 */
struct IndexEntry {
    u_int64_t id{0};
    u_int64_t blockOffsetInRawFile{0};
    u_int64_t startingLineInEntry{0};
    u_int64_t compressedDictionarySize{0};
    u_int32_t bits{0};
    u_int16_t offsetOfFirstValidLine{0};
    Bytef window[WINDOW_SIZE]{0};

    IndexEntry(u_int64_t id,
               u_int32_t bits,
               u_int16_t offsetOfFirstValidLine,
               u_int64_t offsetInRawFile,
               u_int64_t startingLineInEntry);

    explicit IndexEntry(u_int64_t id) {
        this->id = id;
    }

    IndexEntry() = default;

    bool operator==(const IndexEntry &rhs) const;

    bool operator!=(const IndexEntry &rhs) const { return !(rhs == *this); }

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
    u_int64_t blockID{0};

    /**
     * Block offset in raw gz file. Note, that this is not the absolute offset but a relative offset to the last entry.
     * That said, you need to read in the whole index file, before you can effectively use it.
     */
    u_int64_t blockOffsetInRawFile{0};

    /**
     * The id of the first valid line in the block / entry. Note, that this value is relative to the last entry. That
     * said, you need to read in the whole index file, before you can effectively use it.
     */
    u_int64_t startingLineInEntry{0};

    /**
     * Does this block / entry start with a fresh line? If so, offset is zero, otherwise the position of the first line.
     */
    u_int32_t offsetOfFirstValidLine{0};

    unsigned char bits{0};

    unsigned char reserved{0};

    /**
     * Size of the compressed dictionary data in Byte. See below.
     *
     * If the value is 0, no compression was applied. Otherwise it shows the amount of compressed bytes.
     *
     * We know, that the WINDOW_SIZE is 32k, so the compressed dictionary size will be less than 65k. An unsigned int 16
     * will allow us to store this and keep our byte-padding
     */
    u_int16_t compressedDictionarySize{0};

    /**
     * The dictionary is a chunk of uncompressed data from the data block before the block to which this entry points.
     * Before we decompress in raw mode, we need to initialize zlib with this dictionary, otherwise the decompression
     * either fails or, what's worse, produces garbage. In comparison to the zran.c example by Marc Adler, we don't copy
     * around decompressed data but use the more speaking inflateGetDictionary method.
     *
     * As in zindex by Matt Godbolt, we will compress the window before storing it in a file. This makes storage a bit
     * more complicated, but we save around 60% of storage space. This is for a future step.
     */
    Bytef dictionary[WINDOW_SIZE]{0};

    IndexEntryV1(unsigned char bits,
                 u_int64_t blockID,
                 u_int32_t offsetOfFirstValidLine,
                 u_int64_t offsetInRawFile,
                 u_int64_t startingLineInEntry) :
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

    shared_ptr<IndexEntry> toIndexEntry();
};


#endif //FASTQINDEX_COMMONSTRUCTS_H
