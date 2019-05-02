/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_ACTUALRUNNER_H
#define FASTQINDEX_ACTUALRUNNER_H

#include "InputSource.h"
#include "Runner.h"
#include <experimental/filesystem>
#include <string>

using namespace std;
using experimental::filesystem::path;

/**
 * ActualRunner is a base class for Runners which will (actually) perform operations on FASTQ files (index / extract)
 */
class ActualRunner : public Runner {

protected:

    /**
     * The fastq file to work with. The reason, why this is of type InputSource and indexFile is not is, that the
     * fastqFile can be piped in. All other sources (and results) are not pipeable or the piping (cout) is so trivial,
     * that no other class or complex mechanism is necessary.
     */
    shared_ptr<InputSource> fastqFile = shared_ptr<InputSource>(nullptr);

    /**
     * The index file to work with.
     */
    path indexFile;

    ActualRunner(const path &fastqfile, const path &indexfile);

    ActualRunner(istream *fastqStream, const path &indexfile);

    /**
     * Only
     * @param fastqfile
     * @param indexfile
     */
    ActualRunner(const shared_ptr<InputSource> &fastqfile, const path &indexfile);

public:

    /**
     * Used to check
     * @return
     */
    bool checkPremises() override;

    /**
     * This is only valid for the Indexer. The Extractor cannot extract from piped input, as we need to hop around
     * randomly in it.
     * @return
     */
    virtual bool allowsReadFromStreamedSource() { return false; };

    shared_ptr<InputSource> getFastqFile() { return fastqFile; }

    path getIndexFile() { return indexFile; }

};


#endif //FASTQINDEX_ACTUALRUNNER_H
