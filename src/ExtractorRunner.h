/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTORRUNNER_H
#define FASTQINDEX_EXTRACTORRUNNER_H

#include "ActualRunner.h"
#include "Extractor.h"
#include "PathInputSource.h"

/**
 * The ExtractorRunner will, once started, read in the IndexHeader of an FASTQ index file and, based on the encoded
 * indexer version, run the appropriate Extractor version to read in an index file.
 * This modular layout could maybe also be used in another step to convert old indices to new ones.
 */
class ExtractorRunner : public ActualRunner {
protected:

    ExtractMode mode;

    u_int64_t start;

    u_int64_t count;

    uint extractionMultiplier;

    bool enableDebugging;

    shared_ptr<Extractor> extractor;

public:
    /**
     *
     * @param fastqfile         The file to extract from.
     * @param indexfile         The index which is used for extraction.
     * @param resultfile        The file which shall be written or - for stdout.
     * @param forceOverwrite    Overwrite an existing resultfile, if the result is written to a file.
     * @param mode              Mode of operation (either line or segment based)
     * @param start             Sets the either the starting line to extract from OR the segment to extract.
     * @param count             Either sets the count of lines OR the number of segments in total.
     * @param enableDebugging   Used for debugging with e.g. an IDE and for unit tests.
     */
    ExtractorRunner(
            const shared_ptr<PathInputSource> &fastqfile,
            const path &indexfile,
            const path &resultfile,
            bool forceOverwrite,
            ExtractMode mode,
            u_int64_t start,
            u_int64_t count,
            uint extractionMultiplier,
            bool enableDebugging = false
    );

    bool isExtractor() override { return true; };

    bool checkPremises() override;

    unsigned char run() override;

    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_EXTRACTORRUNNER_H
