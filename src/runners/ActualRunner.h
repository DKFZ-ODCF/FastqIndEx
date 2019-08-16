/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#pragma once

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
     * sourceFile can be piped in. All other sources (and results) are not pipeable or the piping (cout) is so trivial,
     * that no other class or complex mechanism is necessary.
     */
    shared_ptr<Source> sourceFile;

    /**
     * For index mode (writing)
     * @param sourceFile
     * @param indexFile
     */
    explicit ActualRunner(const shared_ptr<Source> &sourceFile) {
        this->sourceFile = sourceFile;
    }

public:

    ~ActualRunner() override = default;

    /**
     * Used to check
     * @return
     */
    bool fulfillsPremises() override {

        // Fastq needs to be a (pipe AND piping allowed) or an ((existing file OR symlink with a file) AND readable)
        bool fastqIsValid = false;

        if (sourceFile->isFile()) {
            if (!sourceFile->exists()) {
                addErrorMessage("Source file '" + sourceFile->toString() + "' does not exist.");
            } else {
                fastqIsValid = true;
            }
        } else {
            if (!allowsReadFromStreamedSource()) {
                addErrorMessage("You are not allowed to use piped input for the current mode.");
            } else {
                fastqIsValid = true;
            }
        }

        return fastqIsValid;
    }

    /**
     * This is only valid for the Indexer. The Extractor cannot extract from piped input, as we need to hop around
     * randomly in it.
     * @return
     */
    virtual bool allowsReadFromStreamedSource() { return false; };

    shared_ptr<Source> getSourceFile() { return sourceFile; }

};
