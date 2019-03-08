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
#include <boost/shared_ptr.hpp>
#include <zlib.h>

using boost::shared_ptr;

class Extractor : public ZLibBasedFASTQProcessorBaseClass {

private:

    /**
     * Reader instance which will be used to read in the index.
     */
    boost::shared_ptr<IndexReader> indexReader = boost::shared_ptr<IndexReader>(nullptr);

    u_int64_t startingLine;

    u_int64_t lineCount;

public:
    explicit Extractor(const path &fastqfile,
                       const path &indexfile,
                       u_int64_t startingLine,
                       u_int64_t lineCount,
                       bool enableDebugging);

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

};


#endif //FASTQINDEX_EXTRACTORV1_H
