/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXREADER_H
#define FASTQINDEX_INDEXREADER_H

#include "CommonStructsAndConstants.h"
#include "IndexProcessor.h"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace boost::filesystem;
using namespace boost::interprocess;

/**
 * Class for reading an index file. The current class concept will not feature templating or subclasses, see the comment
 * for readIndexEntryV1 for more details.
 */
class IndexReader : public IndexProcessor {

private:


    /**
     * If the header wasn't read, you are not allowed to read entries.
     * If it was read, you are not allowed to read it again.
     */
    bool headerWasRead = false;

    /**
     * Stream is open, reader is active.
     */
    bool readerIsOpen = false;

    /**
     * Weird behaviour, but I could not get this running with a shared_ptr.
     * This threw boost assertion errors for null pointers.
     */
    boost::filesystem::ifstream *inputStream;

    /**
     * Count of indices which can still be read from the index file. The number is calculated by utilizing the
     * IndexEntryV[n] size.
     */
    ulong indicesLeft{0};

    /**
     * Putting this into a smart pointer always raised: "Assertion `px != 0' failed" during object construction. I do
     * not know, why this happened, but I do a workaround by not using a smart pointer (or any pointer).
     */
    IndexHeader readHeader;

    /**
     * Reads the header (and stores it internally in readHeader). If the header was already read, the existing entry
     * will be returned. Not available for public use, automatically read in tryOpen...
     * @return Either the newly read header or the already stored one.
     */
    IndexHeader readIndexHeader();

public:

    explicit IndexReader(const path &indexFile);

    virtual ~IndexReader();

    /**
     * Try to aquire a lock for the indexfile and open it. Read the header from the file, if possible and
     * Stores error messages for failures.
     * @return true, if everything worked fine.
     */
    bool tryOpenAndReadHeader();

    /**
     * Call this method to read in the whole index file and convert it to a usable form.
     * @return A vector of IndexLine instances.
     */
    vector<IndexLine> readIndexFile();

    /**
     * Specialized version for readIndexFile for V1. Will be called by readIndexFile(), don't call directly.
     * @return A vector of IndexLine instances.
     */
    vector<IndexLine> readIndexFileV1();

    // Example for further versions.
    // vector<IndexLine> readIndexFileV2();
    // vector<IndexLine> readIndexFileV3();

    /**
     * In Java programs, I'd go for more generics or polymorphism. But this is a bit trickier in C++, so for now, we'll
     * stick to several readIndexEntryV[n] methods, assuming, that we will not change the index format to often. If so
     * we could also implement subclasses of IndexReader later.
     * @return
     */
    boost::shared_ptr<IndexEntryV1> readIndexEntryV1();

    // Example for further versions.
    // boost::shared_ptr<IndexEntryV1> readIndexEntryV2();
    // boost::shared_ptr<IndexEntryV1> readIndexEntryV3();

    IndexHeader getIndexHeader() { return readHeader; }

    ulong getIndicesLeft() { return indicesLeft; }
};


#endif //FASTQINDEX_INDEXREADER_H
