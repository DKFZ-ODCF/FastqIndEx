/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <cstdio>
#include "Extractor.h"
#include "ZLibBasedFASTQProcessorBaseClass.h"

ZLibBasedFASTQProcessorBaseClass::ZLibBasedFASTQProcessorBaseClass(
        shared_ptr<InputSource> fastq,
        path index,
        const bool enableDebugging) :
        enableDebugging(enableDebugging) {
    // clang-tidy will complain, that zStream is not initialized by this constructor. This is done in another step.

    fastqfile = move(fastq);
    indexfile = move(index);
}

bool ZLibBasedFASTQProcessorBaseClass::initializeZStream(int mode) {
    zStream.zalloc = nullptr;
    zStream.zfree = nullptr;
    zStream.opaque = nullptr;
    zStream.next_in = nullptr;
    zStream.avail_in = 0;
    zStream.avail_out = 0;

    zlibResult = inflateInit2(&zStream, mode);
    if (zlibResult != Z_OK) {
        addErrorMessage("The zlib stream could not be initialized.");
        return false;
    }

    return true;
}

bool ZLibBasedFASTQProcessorBaseClass::readCompressedDataFromInputSource() {
    /* get some compressed data from input file */
    int result = this->fastqfile->read(input, CHUNK_SIZE);
    if (result == -1) {
        this->addErrorMessage("There was an error during fread.");
        return false;
    } else
        zStream.avail_in = (uInt) result;

    if (zStream.avail_in == 0) {
        this->addErrorMessage("There was no data available in the stream");
        return false;
    }
    zStream.next_in = input;
    return true;
}

/**
 * If at end of block, consider adding an index entry (note that if
 * data_type indicates an end-of-block, then all of the
 * uncompressed data from that block has been delivered, and none
 * of the compressed data after that block has been consumed,
 * except for up to seven bits) -- the total_bytes_out == 0 provides an
 * entry point after the zlib or gzip header, and assures that the
 * index always has at least one access point; we avoid creating an
 * access point after the offset block by checking bit 6 of data_type
 *
 * @param strm z_stream reference to the struct instance used for decompression
 * @return true, if at end of block.
 */
bool ZLibBasedFASTQProcessorBaseClass::checkStreamForBlockEnd() {
    // Also here, clang-tidy complains about bitwise operations on signed fields.
    // However, the code is from Mark Adlers code and I do not want to change it.
    return (zStream.data_type & 128) != 0 && !(zStream.data_type & 64) != 0;
}

/**
 * If the window buffer is full (the buffer with extracted data), we need to reset it.
 * The methods checks and, if necessary, does this.
 * @param zStream
 * @param window
 */
void ZLibBasedFASTQProcessorBaseClass::checkAndResetSlidingWindow() {
    if (zStream.avail_out == 0) {
        zStream.avail_out = WINDOW_SIZE;
        zStream.next_out = window;
    }
}

bool ZLibBasedFASTQProcessorBaseClass::decompressNextChunkOfData(bool checkForStreamEnd, int flushMode) {
    // Inflate until out of input, output, or at end of block
    // Update the total input and output counters
    u_int64_t availableInBeforeInflate = zStream.avail_in;
    u_int64_t availableOutBeforeInflate = zStream.avail_out;
    u_int32_t windowPositionBeforeInflate = WINDOW_SIZE - zStream.avail_out;

    zlibResult = inflate(&zStream, flushMode);
    u_int64_t readBytes = availableInBeforeInflate - zStream.avail_in;
    u_int64_t writtenBytes = availableOutBeforeInflate - zStream.avail_out;// - windowPositionBeforeInflate;

    totalBytesIn += readBytes;
    totalBytesOut += writtenBytes;

    // The window buffer used by inflate will be filled at somehwere between 0 <= n <= WINDOW_SIZE
    // as we work with a string append method, we need to copy the read data to a fresh buffer first.
    Bytef cleansedWindowForCout[CLEAN_WINDOW_SIZE]{0}; // +1 for a definitely 0-terminated c-string!
    std::memcpy(cleansedWindowForCout, window + windowPositionBeforeInflate, writtenBytes);

    if (zlibResult == Z_NEED_DICT) {
        zlibResult = Z_DATA_ERROR;
    }
    if (zlibResult == Z_MEM_ERROR || zlibResult == Z_DATA_ERROR) {
        errorWasRaised = true;
        return false;
    }
    if (checkForStreamEnd && zlibResult == Z_STREAM_END) {
        return false;
    }

    currentDecompressedBlock << cleansedWindowForCout;
    return true;
}
