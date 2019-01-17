/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXER_H
#define FASTQINDEX_INDEXER_H

#include "CommonStructs.h"
#include <string>
#include <boost/filesystem.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/make_shared.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::ptr_container;

class Indexer {

private:

    path fastq;

    path index;

    bool finishedSuccessful = false;

    long foundEntries = 0;

    bool debuggingEnabled;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * keeps the index header
     */
    boost::shared_ptr<IndexHeader> storedHeader;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * Keeps all generated index entries
     */
    boost::shared_ptr<boost::ptr_list<IndexEntry>> storedEntries;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * Keeps all lines read from the fastq file.
     */
    boost::shared_ptr<boost::ptr_list<string>> storedLines;

public:

    static const unsigned int CHUNK_SIZE;

    static const unsigned int WINDOW_SIZE;

    static const unsigned int INDEXER_VERSION;

    Indexer(const path &fastq, const path &index, bool enableDebugging = false);

    bool isDebuggingEnabled() { return debuggingEnabled; }

    path getFastq() { return fastq; }

    path getIndex() { return index; }


    boost::shared_ptr<IndexHeader> createHeader();

    bool createIndex();

    bool wasSuccessful() { return finishedSuccessful; };

    long getFoundEntries() { return foundEntries; };


    boost::shared_ptr<IndexHeader> getStoredHeader() { return storedHeader; };

    const boost::shared_ptr<boost::ptr_list<IndexEntry>> &getStoredEntries() { return storedEntries; }

    const boost::shared_ptr<boost::ptr_list<string>> &getStoredLines() { return storedLines; }
};


#endif //FASTQINDEX_INDEXER_H
