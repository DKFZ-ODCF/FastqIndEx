/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTORV1_H
#define FASTQINDEX_EXTRACTORV1_H


#include "BaseExtractor.h"
#include "CommonStructsAndConstants.h"
#include "ErrorAccumulator.h"
#include "IndexReader.h"
#include <boost/shared_ptr.hpp>
#include <zlib.h>

class Extractor : public BaseExtractor {

private:

    path fastqfile;

    path indexfile;

    /**
     * Store debug information, if this is set to true.
     */
    bool enableDebugging;

    /**
     * Reader instance which will be used to read in the index.
     */
    boost::shared_ptr<IndexReader> indexReader = boost::shared_ptr<IndexReader>(nullptr);

    /**
     * If debugging is enabled, this will hold a copy of all strings extracted from the FASTQ
     */
    vector<string> storedLines;

    uint totalBytesIn{0};

    uint totalBytesOut{0};

    z_stream zStream;

    ulong startingLine;

    ulong lineCount;

public:

    explicit Extractor(path fastqfile, path indexfile, ulong startingLine, ulong lineCount, bool enableDebugging);

    virtual ~Extractor();

    /**
     * Will call tryOpenAndReadHeader on the internal indexReader.
     */
    bool checkPremises();

    bool initializeZStream(z_stream *const strm);

    bool readCompressedDataFromStream(FILE *const inputFile, z_stream *const strm, Byte *const buffer);

    /**
     * For now directly to cout?
     * @param start
     * @param count
     */
    bool extractReadsToCout();

    const vector<string> &getStoredLines() { return storedLines; };
};


#endif //FASTQINDEX_EXTRACTORV1_H
