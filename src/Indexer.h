/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXER_H
#define FASTQINDEX_INDEXER_H

#include "CommonStructsAndConstants.h"
#include "ErrorAccumulator.h"
#include "IndexWriter.h"
#include <string>
#include <boost/filesystem.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/make_shared.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::ptr_container;

/**
 * The Indexer class is used to walk through a gz compressed FASTQ file and to write an index for this file.
 */
class Indexer : public ErrorAccumulator {

private:

    path fastq;

    path index;

    bool finishedSuccessful = false;

    long numberOfFoundEntries = 0;

    bool debuggingEnabled;

    boost::shared_ptr<IndexWriter> indexWriter;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * keeps the index header
     */
    boost::shared_ptr<IndexHeader> storedHeader = boost::shared_ptr<IndexHeader>(nullptr);

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * Keeps all generated index entries
     */
    boost::shared_ptr<boost::ptr_list<IndexEntryV1>> storedEntries;

    /**
     * For debug and test purposes, used when debuggingEnabled is true
     * Keeps all lines read from the fastq file.
     */
    boost::shared_ptr<boost::ptr_list<string>> storedLines;

public:

    static const unsigned int CHUNK_SIZE;

    static const unsigned int WINDOW_SIZE;

    /**
     * This is the version of the current Indexer implementation. In contrary to the Extractor, there is always only one
     * Indexer version available.
     * If the Indexer ever changes, keep in mind to increment this version! This will be used to select the appropriate
     * Extractor class when an index is read!
     */
    static const unsigned int INDEXER_VERSION;

    /**
     * Be careful, when you enableDebuggin. This will tell the Indexer to store information about the process, which can
     * e.g. be used for unit test. E.g. this will store ALL lines found in the FASTQ file! So it is absolutely not
     * advisable to use it with large FASTQ files. For test data it is safe to use.
     * @param fastq The FASTQ we are working on.
     * @param index The index for the FASTQ.
     * @param enableDebugging Store debug information or not.
     */
    Indexer(const path &fastq, const path &index, bool enableDebugging = false);

    bool checkPremises();

    /**
     * Overriden to also pass through (copywise, safe but slow but also only with a few entries and in error cases)
     * error messages from the used IndexWriter instance.
     * @return Merged vector of the objects error messages + the index writers error messages.
     */
    vector<string> getErrorMessages() override;

    bool isDebuggingEnabled() { return debuggingEnabled; }

    path getFastq() { return fastq; }

    path getIndex() { return index; }

    /**
     * This will create an IndexHeader instance with INDEXER_VERSION and the size of the used IndexEntry struct.
     */
    boost::shared_ptr<IndexHeader> createHeader();

    /**
     * Start the index creation,
     * @return true, if everything went fine.
     */
    bool createIndex();

    bool wasSuccessful() { return finishedSuccessful; };

    long getFoundEntries() { return numberOfFoundEntries; };


    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * @return The IndexHeader, which was created during createIndex()
     */
    boost::shared_ptr<IndexHeader> getStoredHeader() { return storedHeader; };

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * @return The index entries, which were created during the index run.
     */
    const boost::shared_ptr<boost::ptr_list<IndexEntryV1>> &getStoredEntries() { return storedEntries; }

    /**
     * For debugging, works only, when enableDebugging was true on object construction.
     * Be sure what you do, before you turn on enableDebugging! This will return ALL lines found in the FASTQ file!
     */
    const boost::shared_ptr<boost::ptr_list<string>> &getStoredLines() { return storedLines; }
};


#endif //FASTQINDEX_INDEXER_H
