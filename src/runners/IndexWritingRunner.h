/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

//#ifndef FASTQINDEX_INDEXWRITINGRUNNER_H
//#define FASTQINDEX_INDEXWRITINGRUNNER_H

#pragma once

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

    IndexWritingRunner(const shared_ptr<Source> &sourceFile,
                       const shared_ptr<Sink> &indexFile) :
            ActualRunner(sourceFile) {
        this->indexFile = indexFile;
    }

    ~IndexWritingRunner() override = default;

    shared_ptr<Sink> getIndexFile() { return indexFile; }

    bool fulfillsPremises() override{

        bool fastqIsValid = ActualRunner::fulfillsPremises();
        bool indexIsValid = indexFile->fulfillsPremises();
//    // Index files are automatically overwritten but need to have write access!
//    bool indexIsValid = true;
//    // It is totally ok, if the index does not exist. We'll create it then.
//    if (exists(inputIndexFile)) {
//        if (is_symlink(inputIndexFile))
//            inputIndexFile = read_symlink(inputIndexFile);
//
//        if (!is_regular_file(inputIndexFile)) {
//            indexIsValid = false;
//            addErrorMessage(ERR_MESSAGE_INDEX_INVALID);
//        }
//    }

        return fastqIsValid && indexIsValid;
    }

    vector<string> getErrorMessages() override{
        auto a = ErrorAccumulator::getErrorMessages();
        auto b = sourceFile->getErrorMessages();
        auto c = indexFile->getErrorMessages();
        return concatenateVectors(a, b, c);
    }

};


//bool IndexWritingRunner::fulfillsPremises()

//vector<string> IndexWritingRunner::getErrorMessages()
//#endif //FASTQINDEX_INDEXWRITINGRUNNER_H
