/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTORRUNNER_H
#define FASTQINDEX_EXTRACTORRUNNER_H

#include "runners/ActualRunner.h"
#include "runners/IndexReadingRunner.h"
#include "process/extract/Extractor.h"
#include "process/io/FileSource.h"

/**
 * The ExtractorRunner will, once started, read in the IndexHeader of an FASTQ index file and, based on the encoded
 * indexer version, run the appropriate Extractor version to read in an index file.
 * This modular layout could maybe also be used in another step to convert old indices to new ones.
 */
class ExtractorRunner : public IndexReadingRunner {
protected:

    ExtractMode mode;

    int64_t start;

    int64_t count;

    uint recordSize;

    bool enableDebugging;

    shared_ptr<Extractor> extractor;

public:
    /**
     *
     * @param sourceFile         The file to extract from.
     * @param indexFile         The index which is used for extraction.
     * @param resultFile        The file which shall be written or - for stdout.
     * @param forceOverwrite    Overwrite an existing resultfile, if the result is written to a file.
     * @param mode              Mode of operation (either line or segment based)
     * @param start             Sets the either the starting line to extract from OR the segment to extract.
     * @param count             Either sets the count of lines OR the number of segments in total.
     * @param enableDebugging   Used for debugging with e.g. an IDE and for unit tests.
     */
    ExtractorRunner(
            const shared_ptr<Source> &sourceFile,
            const shared_ptr<Source> &indexFile,
            const shared_ptr<Sink> &resultFile,
            bool forceOverwrite,
            ExtractMode mode,
            int64_t start,
            int64_t count,
            uint recordSize,
            bool enableDebugging = false
    );

    shared_ptr<Sink> getResultSink() {
        return extractor->getResultSink();
    }

    bool isExtractor() override { return true; };

    bool fulfillsPremises() override;

    unsigned char _run() override;

    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_EXTRACTORRUNNER_H
