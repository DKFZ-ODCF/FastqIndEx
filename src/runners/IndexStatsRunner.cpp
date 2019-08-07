/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexStatsRunner.h"

IndexStatsRunner::IndexStatsRunner(const shared_ptr<Source> &indexFile, int start, int amount) : IndexReadingRunner(
        shared_ptr<Source>(nullptr), indexFile) {

    this->indexReader = make_shared<IndexReader>(indexFile);
    this->start = start;
    this->amount = amount;
}

bool IndexStatsRunner::allowsReadFromStreamedSource() {
    return true;
}

bool IndexStatsRunner::fulfillsPremises() {
    // There is no FASTQ file here
//    auto res = ActualRunner::fulfillsPremises();
    auto res2 = this->indexReader->tryOpenAndReadHeader();
    return res2;
}

unsigned char IndexStatsRunner::_run() {
    auto header = this->indexReader->getIndexHeader();
    auto indicesLeft = this->indexReader->getIndicesLeft();

    cout << "Statistics for index file " << this->indexFile->toString() << "\n";
    cout << "\tMagic number:     " << header.magicNumber << "\n";
    cout << "\tWriter version:   " << header.indexWriterVersion << "\n";
    cout << "\tBlock Interval:   " << header.blockInterval << "\n";
    if (header.dictionariesAreCompressed) {
        cout << "\tIndex entry size: <n/a:Dictionary compression is active>\n";
    } else {
        cout << "\tIndex entry size: " << header.sizeOfIndexEntry << " Byte\n";
    }
    cout << "\tIndex entries:    " << indicesLeft << "\n";
    cout << "\tLines in file:    " << header.linesInIndexedFile << "\n";

    for (int i = 0; i < start; i++)
        this->indexReader->readIndexEntry();

    int toRead = this->indexReader->getIndicesLeft();
    if (amount > 0) {
        toRead = amount;
    }

    if (this->indexReader->getIndicesLeft() == 0) {
        cout << "Starting index id exceeds entry count.\n";
        return 1;
    } else {
        for (int i = 0; i < toRead && this->indexReader->getIndicesLeft() > 0; i++) {
            auto entry = this->indexReader->readIndexEntry();

            printIndexEntryToConsole(entry, (i + start));
        }
        return 0;
    }
}

void IndexStatsRunner::printIndexEntryToConsole(const shared_ptr<IndexEntry> &entry, u_int64_t entryNumber,
                                                bool toCErr) {
    auto &_ostream = toCErr ? cout : cerr;
    _ostream << "Entry number:    " << entryNumber << "\n";
    _ostream << "  Entry id:      " << entry->id << "\n";
    _ostream << "  Raw offset:    " << entry->blockOffsetInRawFile << "\n";
    _ostream << "  Starting line: " << entry->startingLineInEntry << "\n";
    _ostream << "    Record (/4): " << (entry->startingLineInEntry / 4) << "\n";
    _ostream << "  Line offset:   " << entry->offsetToNextLineStart << "\n";
    _ostream << "  Bits:          " << entry->bits << "\n";
}

vector<string> IndexStatsRunner::getErrorMessages() {
    vector<string> l = ErrorAccumulator::getErrorMessages();
    vector<string> r = indexReader->getErrorMessages();
    return mergeToNewVector(l, r);
}

