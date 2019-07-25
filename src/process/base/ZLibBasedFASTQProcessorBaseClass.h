/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_ZLIBHELPER_H
#define FASTQINDEX_ZLIBHELPER_H

#include "common/CommonStructsAndConstants.h"
#include "common/ErrorAccumulator.h"
#include "process/extract/IndexReader.h"
#include "process/io/Source.h"
#include "process/io/Sink.h"
#include "process/io/PathSource.h"
#include <cstdio>
#include <experimental/filesystem>
#include <zlib.h>

using std::experimental::filesystem::path;
using std::stringstream;

/**
 * Helper class for zlib stuff.
 */
class ZLibBasedFASTQProcessorBaseClass : public ErrorAccumulator {

protected:

    shared_ptr<Source> fastqFile;

    shared_ptr<Source> inputIndexFile;

    shared_ptr<Sink> outputIndexFile;

    /**
     * Set to true, as soon as createIndex() was started.
     */
    bool wasStarted = false;

    /**
     * Indicates that an error was raised within index or extract.
     */
    bool errorWasRaised = false;

    /**
     * Indicates, whether index / extract was successful.
     */
    bool finishedSuccessful = false;

    /**
     * The z_stream instance used by index / extract.
     */
    z_stream zStream;

    /**
     * Input buffer which holds compressed data.
     */
    Byte input[CHUNK_SIZE]{0};

    /**
     * Sliding window which holds decompressed data.
     */
    Byte window[WINDOW_SIZE]{0};

    /**
     * Store debug information, if this is set to true.
     */
    bool enableDebugging;

    /**
     * If debugging is enabled, this will hold a copy of all strings extracted from the FASTQ
     */
    vector<string> storedLines;

    /**
     * The total amount of Bytes read from the FASTQ file. 8 Byte to avoid 4 Byte integer overflow of zlib.
     */
    u_int64_t totalBytesIn{0};

    /**
     * The total amount of Bytes written to the internal buffer / window. 8 Byte to avoid 4 Byte integer overflow of
     * zlib.
     */
    u_int64_t totalBytesOut{0};

    /**
     * Stores results of zlib operations.
     */
    int zlibResult{0};

    /**
     * Ignore the first block! Does not contain any data. Effectively tells the indexer / extractor, that the first
     * block was not yet processed.
     *
     */
    bool firstPass = true;

    /**
     * Decompressed contents of the current processed block.
     */
    stringstream currentDecompressedBlock;

    ZLibBasedFASTQProcessorBaseClass(shared_ptr<Source> fastq, shared_ptr<Source> index, bool enableDebugging);

    ZLibBasedFASTQProcessorBaseClass(shared_ptr<Source> fastq, shared_ptr<Sink> index, bool enableDebugging);

public:

    shared_ptr<Source> getFastq() { return fastqFile; }

    auto getInputIndexFile() { return inputIndexFile; }

    auto getOutputIndexFile() { return outputIndexFile; }

    bool wasSuccessful() { return finishedSuccessful; };

    bool isDebuggingEnabled() { return enableDebugging; }

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * Be sure what you do, before you turn on enableDebugging! This will return ALL lines found in the FASTQ file!
     */
    const vector<string> &getStoredLines() { return storedLines; }

    /**
     * Will create a struct instance of type z_stream, initialise some fields like to be seen in
     *
     * @see https://github.com/madler/zlib/blob/master/examples/zran.c
     * @return true, if the strm initialize was successful, otherwise false.
     */
    bool initializeZStream(int mode);

    /**
     * Intializes the z_stream with 47 like seen in zran.c for automatic zlib or libz decoding
     */
    bool initializeZStreamForInflate() {
        return initializeZStream(47);
    }

    /**
     * Intializes the z_stream with -15 like seen in zran.c for raw / random inflate
     */
    bool initializeZStreamForRawInflate() {
        return initializeZStream(-15);
    }

    /**
     * Read data from an input source and store it to the internal input buffer and zstream instance.
     *
     * This version is merely for debugg
     *
     * @param inputFile File to read from (FASTQ)
     * @param strm Reference to the strm struct which is used by zlib decompression.
     * @param buffer which will hold the latest decompressed chunk of data.
     * @return
     */
    bool readCompressedDataFromSource();

    bool checkStreamForBlockEnd();

    void checkAndResetSlidingWindow();

    bool decompressNextChunkOfData(bool checkForStreamEnd, int flushMode);

    void clearCurrentCompressedBlock();
};


#endif //FASTQINDEX_ZLIBHELPER_H
