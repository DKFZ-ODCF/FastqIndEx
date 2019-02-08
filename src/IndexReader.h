/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
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
 * Class for reading an index file.
 */
class IndexReader : public IndexProcessor {

private:

    bool headerWasRead = false;

    explicit IndexReader(const path &indexFile);

    bool open();

    // Weird behaviour, but I could not get this running with a shared_ptr.
    // This threw boost assertion errors for null pointers.
    boost::filesystem::ifstream *inputStream;

    long indicesLeft;

public:

    static boost::shared_ptr<IndexReader> create(const path &indexFile);

    virtual ~IndexReader();

    boost::shared_ptr<IndexHeader> readIndexHeader();

    boost::shared_ptr<IndexEntryV1> readIndexEntry();

    long getIndicesLeft() { return indicesLeft; }
};


#endif //FASTQINDEX_INDEXREADER_H
