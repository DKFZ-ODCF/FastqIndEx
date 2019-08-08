/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXREADER_H
#define FASTQINDEX_INDEXREADER_H

#include "common/CommonStructsAndConstants.h"
#include "common/ErrorAccumulator.h"
#include "common/IndexHeader.h"
#include "common/IndexEntry.h"
#include "process/io/locks/PathLockHandler.h"
#include "process/io/Source.h"
#include <experimental/filesystem>
#include <fstream>


/**
 * Class for reading an index file. The current class concept will not feature templating or subclasses, see the comment
 * for readIndexEntryV1 for more details.
 */
class IndexReader : public ErrorAccumulator {

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

    shared_ptr<Source> indexFile;

    /**
     * Count of indices which can still be read from the index file. The number is calculated by utilizing the
     * IndexEntryV[n] size.
     */
    int64_t indicesLeft{0};

    int64_t indicesCount{0};
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

    explicit IndexReader(const shared_ptr<Source> &indexFile);

    ~IndexReader() override;

    /**
     * Try to aquire a lock for the indexFile and open it. Read the header from the file, if possible and
     * Stores error messages for failures.
     * @return true, if everything worked fine.
     */
    bool tryOpenAndReadHeader();

    /**
     * Call this method to read in the whole index file and convert it to a usable form.
     * @return A vector of IndexLine instances.
     */
    vector<shared_ptr<IndexEntry>> readIndexFile();

    /**
     * Specialized version for readIndexFile for V1. Will be called by readIndexFile(), don't call directly.
     * @return A vector of IndexLine instances.
     */
    vector<shared_ptr<IndexEntryV1>> readIndexFileV1();

    /**
     * Reads one processed index entry from the file. Effectively calls readIndexEntryV[n].
     * @return
     */
    shared_ptr<IndexEntry> readIndexEntry();

    // Example for further versions.
    // vector<IndexEntry> readIndexFileV2();
    // vector<IndexEntry> readIndexFileV3();

    /**
     * In Java programs, I'd go for more generics or polymorphism. But this is a bit trickier in C++, so for now, we'll
     * stick to several readIndexEntryV[n] methods, assuming, that we will not change the index format to often. If so
     * we could also implement subclasses of IndexReader later.
     * @return
     */
    shared_ptr<IndexEntryV1> readIndexEntryV1();

    // Example for further versions.
    // shared_ptr<IndexEntryV1> readIndexEntryV2();
    // shared_ptr<IndexEntryV1> readIndexEntryV3();

    IndexHeader getIndexHeader() { return readHeader; }

    int64_t getIndicesLeft() { return indicesLeft; }

};


#endif //FASTQINDEX_INDEXREADER_H
