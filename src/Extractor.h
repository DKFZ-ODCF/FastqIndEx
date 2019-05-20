/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_EXTRACTORV1_H
#define FASTQINDEX_EXTRACTORV1_H

#include "CommonStructsAndConstants.h"
#include "ErrorAccumulator.h"
#include "IndexReader.h"
#include "ZLibBasedFASTQProcessorBaseClass.h"
#include "PathInputSource.h"
#include <experimental/filesystem>
#include <zlib.h>

using namespace std;
using experimental::filesystem::path;

class Extractor : public ZLibBasedFASTQProcessorBaseClass {

private:

    /**
     * Reader instance which will be used to read in the index.
     */
    shared_ptr<IndexReader> indexReader = shared_ptr<IndexReader>(nullptr);

    u_int64_t startingLine;

    u_int64_t lineCount;

    path resultFile;

    bool useFile{false};

    /**
     * If the extractor shall write a new fastq file, this indicates, that the file will be overwritten.
     */
    bool forceOverwrite;

public:

    /**
     * @param fastqfile         The file from which we will extract data
     * @param indexfile         The index file for this file
     * @param resultfile        The result file or
     * @param forceOverwrite    If the resultfile exists, we can overwrite it with this flag
     * @param startingLine      Start extraction from this line
     * @param lineCount         Extract a maximum of lineCount lines
     * @param enableDebugging   Used for interactive debugging and unit tests
     */
    explicit Extractor(
            const shared_ptr<PathInputSource> &fastqfile,
            const path &indexfile,
            const path &resultfile,
            bool forceOverwrite,
            u_int64_t startingLine,
            u_int64_t lineCount,
            bool enableDebugging
    );

    virtual ~Extractor() = default;

    /**
     * Will call tryOpenAndReadHeader on the internal indexReader.
     */
    bool checkPremises();

    /**
     * For now directly to cout?
     * @param start
     * @param count
     */
    bool extractReadsToCout();

    /**
     * Overriden to also pass through (copywise, safe but slow but also only with a few entries and in error cases)
     * error messages from the used IndexReader and ZLibHelper instance.
     * @return Merged vector of the objects error messages + the index writers error messages.
     */
    vector<string> getErrorMessages() override;

    void storeOrOutputLine(ostream *outStream, uint64_t *skipLines, string line);
};


#endif //FASTQINDEX_EXTRACTORV1_H
