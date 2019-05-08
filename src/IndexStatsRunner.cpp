/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexStatsRunner.h"

IndexStatsRunner::IndexStatsRunner(const path &indexfile, int start, int amount) : ActualRunner(nullptr, indexfile) {

    this->indexReader = make_shared<IndexReader>(indexfile);
    this->start = start;
    this->amount = amount;
}

bool IndexStatsRunner::allowsReadFromStreamedSource() {
    return true;
}

bool IndexStatsRunner::checkPremises() {
    auto res = ActualRunner::checkPremises();
    auto res2 = this->indexReader->tryOpenAndReadHeader();
    return res && res2;
}

unsigned char IndexStatsRunner::run() {
    auto header = this->indexReader->getIndexHeader();
    auto indicesLeft = this->indexReader->getIndicesLeft();

    cout << "Statistics for index file " << this->indexFile.string() << "\n";
    cout << "\tMagic number:     " << header.magicNumber << "\n";
    cout << "\tWriter version:   " << header.indexWriterVersion << "\n";
    cout << "\tBlock Interval:   " << header.blockInterval << "\n";
    cout << "\tIndex entry size: " << header.sizeOfIndexEntry << "\n";
    cout << "\tIndex entries:    " << indicesLeft << "\n";

    this->indexReader->setPosition(start);
    int toRead = this->indexReader->getIndicesLeft();
    if (amount > 0) {
        toRead = amount;
    }

    if (this->indexReader->getIndicesLeft() == 0) {
        cout << "Starting index id exceeds entry count.\n";
        return 1;
    } else {

        for (int i = 0; i < toRead; i++) {
            auto entry = this->indexReader->readIndexEntry();

            cout << "Entry number:      " << (i + start) << "\n";
            cout << "\tEntry id:      " << entry->id << "\n";
            cout << "\tRaw offset:    " << entry->offsetInRawFile << "\n";
            cout << "\tStarting line: " << entry->startingLineInEntry << "\n";
            cout << "\tLine offset:   " << entry->offsetOfFirstValidLine << "\n";
            cout << "\tBits:          " << entry->bits << "\n";
        }
        return 0;
    }
}

vector<string> IndexStatsRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexReader->getErrorMessages();
    return mergeToNewVector(l, r);
}

