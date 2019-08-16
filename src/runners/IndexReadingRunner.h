/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#pragma once

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

    IndexReadingRunner(const shared_ptr<Source> &sourceFile,
                       const shared_ptr<Source> &indexFile)
            : ActualRunner(sourceFile) {
        this->indexFile = indexFile;
    }


    shared_ptr<Source> getIndexFile() { return indexFile; }

    bool fulfillsPremises() override {
        bool fastqIsValid = ActualRunner::fulfillsPremises();
        bool indexIsValid = indexFile->fulfillsPremises();

        return fastqIsValid && indexIsValid;
    }

    vector<string> getErrorMessages() override{
        if (sourceFile.get()) {
            auto a = ErrorAccumulator::getErrorMessages();
            auto b = sourceFile->getErrorMessages();
            auto c = indexFile->getErrorMessages();
            return concatenateVectors(a, b, c);
        } else {
            auto a = ErrorAccumulator::getErrorMessages();
            auto c = indexFile->getErrorMessages();
            return concatenateVectors(a, c);
        }
    }
};
