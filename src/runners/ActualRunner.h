/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_ACTUALRUNNER_H
#define FASTQINDEX_ACTUALRUNNER_H

#include "process/io/Source.h"
#include "process/io/Sink.h"
#include "Runner.h"
#include <experimental/filesystem>
#include <string>
#include <iostream>

using namespace std;
using experimental::filesystem::path;

/**
 * ActualRunner is a base class for Runners which will (actually) perform operations on FASTQ files (index / extract)
 */
class ActualRunner : public Runner {

protected:

    /**
     * The fastq file to work with. The reason, why this is of type Source and indexFile is not is, that the
     * fastqFile can be piped in. All other sources (and results) are not pipeable or the piping (cout) is so trivial,
     * that no other class or complex mechanism is necessary.
     */
    shared_ptr<Source> fastqFile = shared_ptr<Source>(nullptr);

    /**
     * For index mode (writing)
     * @param fastqfile
     * @param indexFile
     */
    explicit ActualRunner(const shared_ptr<Source> &fastqfile);

public:

    ~ActualRunner() override = default;

    /**
     * Used to check
     * @return
     */
    bool fulfillsPremises() override;

    /**
     * This is only valid for the Indexer. The Extractor cannot extract from piped input, as we need to hop around
     * randomly in it.
     * @return
     */
    virtual bool allowsReadFromStreamedSource() { return false; };

    shared_ptr<Source> getFastqFile() { return fastqFile; }


};

/**
 * For extract and stats, takes an Source for the FQI
 *
 * Normally, I'd say, that this class and the Writing class are not necessary and that the differentiation could be
 * handled with a template super class, but it somehow did not work.
 *
 * Either way, I will try to use this to make more detailed checks possible.
 */
class IndexReadingRunner : public ActualRunner {

protected:

    /**
     * The index file to work with.
     */
    shared_ptr<Source> indexFile;

public:

    IndexReadingRunner(const shared_ptr<Source> &fastqfile,
                       const shared_ptr<Source> &indexFile)
            : ActualRunner(fastqfile) {
        this->indexFile = indexFile;
    }


    shared_ptr<Source> getIndexFile() { return indexFile; }

    bool fulfillsPremises() override;

    vector<string> getErrorMessages() override;
};

/**
 * For index (actually only for index, maybe we'll have another runner later.
 */
class IndexWritingRunner : public ActualRunner {

protected:

    /**
     * The index file for output...
     */
    shared_ptr<Sink> indexFile;

public:

    IndexWritingRunner(const shared_ptr<Source> &fastqfile,
                       const shared_ptr<Sink> &indexFile) :
            ActualRunner(fastqfile) {
        this->indexFile = indexFile;
    }

    ~IndexWritingRunner() override = default;

    shared_ptr<Sink> getIndexFile() { return indexFile; }

    bool fulfillsPremises() override;

    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_ACTUALRUNNER_H
