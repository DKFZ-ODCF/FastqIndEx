/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXWRITER_H
#define FASTQINDEX_INDEXWRITER_H

#include "CommonStructs.h"
#include "IndexProcessor.h"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace boost::filesystem;
using namespace boost::interprocess;

class IndexWriter : public IndexProcessor {

private:

    /**
     * You are not allowed to write an index twice.
     * You are also not allowed to write an entry before the index.
     */
    bool headerWasWritten = false;

    explicit IndexWriter(const path &indexFile);

    bool open();

    boost::shared_ptr<ofstream> outputStream;

public:

    static const unsigned int INDEX_WRITER_VERSION;

    static boost::shared_ptr<IndexWriter> create(const path &indexFile);

    bool writeIndexHeader(boost::shared_ptr<IndexHeader> header);

    bool writeIndexEntry(boost::shared_ptr<IndexEntry> entry);

    void flush();
};


#endif //FASTQINDEX_INDEXWRITER_H
