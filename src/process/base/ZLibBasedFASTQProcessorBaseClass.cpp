/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/FileSource.h"
#include "ZLibBasedFASTQProcessorBaseClass.h"
#include <experimental/filesystem>
#include <iostream>
#include <zlib.h>

ZLibBasedFASTQProcessorBaseClass::ZLibBasedFASTQProcessorBaseClass(
        shared_ptr<Source> fastq,
        shared_ptr<Source> index,
        const bool enableDebugging) :
        enableDebugging(enableDebugging) {

    // clang-tidy will complain, that zStream is not initialized by this constructor. This is done in another step.
    sourceFile = move(fastq);
    inputIndexFile = move(index);
}

ZLibBasedFASTQProcessorBaseClass::ZLibBasedFASTQProcessorBaseClass(
        shared_ptr<Source> fastq,
        shared_ptr<Sink> index,
        const bool enableDebugging) :
        enableDebugging(enableDebugging) {

    // clang-tidy will complain, that zStream is not initialized by this constructor. This is done in another step.
    sourceFile = move(fastq);
    outputIndexFile = move(index);
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

bool ZLibBasedFASTQProcessorBaseClass::readCompressedDataFromSource() {
    /* get some compressed data from input file */
    int64_t result = this->sourceFile->read(input, CHUNK_SIZE);

    if (result == -1) {
        this->addErrorMessage("Could not read source file '", sourceFile->toString(), "'.");
        return false;
    } else
        zStream.avail_in = static_cast<uInt>(result);

    if (zStream.avail_in == 0) {
        this->addErrorMessage("There was no data available in the compressed stream.");
        return false;
    }
    zStream.next_in = input;
    return true;
}

/**
 * At the end of the block, consider adding an index entry (note that if
 * data_type indicates an end-of-block, then all of the
 * uncompressed data from that block has been delivered, and none
 * of the compressed data after that block has been consumed,
 * except for up to seven bits) -- the total_bytes_out == 0 provides an
 * entry point after the zlib or gzip header, and assures that the
 * index always has at least one access point; we avoid creating an
 * access point after the offset block by checking bit 6 of data_type
 *
 * data_type is set by inflate as taken from: https://www.zlib.net/manual.html
 * The Z_BLOCK option assists in appending to or combining deflate streams. To assist in this, on return inflate()
 * always sets strm->data_type to the number of unused bits in the last byte taken from strm->next_in, plus 64 if
 * inflate() is currently decoding the last block in the deflate stream, plus 128 if inflate() returned immediately
 * after decoding an end-of-block code or decoding the complete header up to just before the first byte of the deflate
 * stream.
 *
 * So shortly said: Both 64 and 128 tell you, that no unused bits are leftover.
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
void ZLibBasedFASTQProcessorBaseClass::resetSlidingWindowIfNecessary() {
    if (zStream.avail_out == 0) {
        zStream.avail_out = WINDOW_SIZE;
        zStream.next_out = window;
    }
}

bool ZLibBasedFASTQProcessorBaseClass::decompressNextChunkOfData(bool checkForStreamEnd, int flushMode) {
    // Inflate until out of input, output, or at end of block
    // Update the total input and output counters
    int64_t availableInBeforeInflate = zStream.avail_in;
    int64_t availableOutBeforeInflate = zStream.avail_out;
    u_int32_t windowPositionBeforeInflate = WINDOW_SIZE - zStream.avail_out;

    zlibResult = inflate(&zStream, flushMode);
    int64_t readBytes = availableInBeforeInflate - zStream.avail_in;
    int64_t writtenBytes = availableOutBeforeInflate - zStream.avail_out;

    totalBytesIn += readBytes;
    totalBytesOut += writtenBytes;

    // The window buffer used by inflate will be filled at somewhere between 0 <= n <= WINDOW_SIZE
    // as we work with a string append method, we need to copy the read data to a fresh buffer first.
    Bytef cleansedWindowForCout[CLEAN_WINDOW_SIZE]{
            0}; // +1 more Byte than WINDOW_SIZE for a definitely 0-terminated c-string!
    std::memcpy(cleansedWindowForCout, window + windowPositionBeforeInflate, writtenBytes);// TODO

    if (zlibResult == Z_NEED_DICT) {
        zlibResult = Z_DATA_ERROR;
    }
    if (zlibResult == Z_MEM_ERROR || zlibResult == Z_DATA_ERROR) {
        cerr << "Zlib data or memory error occurred: " << zlibResult << "\n";
        errorWasRaised = true;
        return false;
    }
    if (checkForStreamEnd && zlibResult == Z_STREAM_END) {
        return false;
    }

    currentDecompressedBlock << cleansedWindowForCout;
    return true;
}

void ZLibBasedFASTQProcessorBaseClass::clearCurrentCompressedBlock() {
    currentDecompressedBlock.str("");
    currentDecompressedBlock.clear();
}