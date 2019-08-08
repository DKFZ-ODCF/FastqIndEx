/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXSTATSRUNNER_H
#define FASTQINDEX_INDEXSTATSRUNNER_H


#include "ActualRunner.h"
#include "process/extract/IndexReader.h"

class IndexStatsRunner : public IndexReadingRunner {

private:

    shared_ptr<IndexReader> indexReader;

    int start;

    int amount;

public:

    IndexStatsRunner(const shared_ptr<Source> &indexFile, int start, int amount);

    bool allowsReadFromStreamedSource() override;

    bool fulfillsPremises() override;

    unsigned char _run() override;

    vector<string> getErrorMessages() override;

    static void printIndexEntryToConsole(const shared_ptr<IndexEntry> &entry, int64_t entryNumber, bool toCErr = true);
};


#endif //FASTQINDEX_INDEXSTATSRUNNER_H
