/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXENTRYV1_H
#define FASTQINDEX_INDEXENTRYV1_H

#include "CommonStructsAndConstants.h"
#include "IndexEntry.h"

using namespace std;

/**
 * Represents an entry in a gz index file
 * The order of the entries is set so, that we have a reduced padding!
 * Please take a look here for some explanations:
 *   https://stackoverflow.com/questions/119123/why-isnt-sizeof-for-a-struct-equal-to-the-sum-of-sizeof-of-each-member
 * The size of this struct is 32800 Bytes.
 */
struct IndexEntryV1 : public VirtualIndexEntry {

    /**
     * The identifier of the raw compressed block for which this entry is.
     */
    u_int64_t blockID{0};

    /**
     * Offset / Byte position of the referenced compressed block offset in the raw gz file.
     */
    u_int64_t blockOffsetInRawFile{0};

    /**
     * The id of the first valid line in the block / entry.
     */
    u_int64_t startingLineInEntry{0};

    /**
     * Does this block / entry start with a fresh line? If so, offset is zero, otherwise the position of the first line.
     */
    u_int32_t offsetToNextLineStart{0};

    /**
     * Needed for zlib.
     */
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

    static shared_ptr<IndexEntryV1> from(unsigned char bits,
                                         u_int64_t blockID,
                                         u_int32_t offsetOfFirstValidLine,
                                         u_int64_t offsetInRawFile,
                                         u_int64_t startingLineInEntry) {
        return make_shared<IndexEntryV1>(bits, blockID, offsetOfFirstValidLine, offsetInRawFile, startingLineInEntry);
    }

    IndexEntryV1(unsigned char bits,
                 u_int64_t blockID,
                 u_int32_t offsetOfFirstValidLine,
                 u_int64_t offsetInRawFile,
                 u_int64_t startingLineInEntry) :
            bits(bits),
            blockID(blockID),
            offsetToNextLineStart(offsetOfFirstValidLine),
            blockOffsetInRawFile(offsetInRawFile),
            startingLineInEntry(startingLineInEntry) {}

    IndexEntryV1() = default;

    bool operator==(const IndexEntryV1 &rhs) const {
        return bits == rhs.bits &&
               blockID == rhs.blockID &&
               offsetToNextLineStart == rhs.offsetToNextLineStart &&
               blockOffsetInRawFile == rhs.blockOffsetInRawFile &&
               startingLineInEntry == rhs.startingLineInEntry;
    }

    bool operator!=(const IndexEntryV1 &rhs) const {
        return !(rhs == *this);
    }

    shared_ptr<IndexEntry> toIndexEntry() {
        auto indexLine = std::make_shared<IndexEntry>(
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
};

#endif //FASTQINDEX_INDEXENTRYV1_H
