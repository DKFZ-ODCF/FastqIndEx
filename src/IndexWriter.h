/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXWRITER_H
#define FASTQINDEX_INDEXWRITER_H

#include "CommonStructsAndConstants.h"
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

    bool writerIsOpen = false;

    boost::shared_ptr<boost::filesystem::ofstream> outputStream;

public:

    static const unsigned int INDEX_WRITER_VERSION;

    explicit IndexWriter(const path &indexFile);

    virtual ~IndexWriter();

    bool tryOpen();

    bool writeIndexHeader(boost::shared_ptr<IndexHeader> header);

    bool writeIndexEntry(boost::shared_ptr<IndexEntryV1> entry);

    void flush();

    bool close();
};


#endif //FASTQINDEX_INDEXWRITER_H
