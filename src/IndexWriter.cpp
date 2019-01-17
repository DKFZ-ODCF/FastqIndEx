/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <boost/make_shared.hpp>
#include <boost/filesystem/fstream.hpp>
#include "IndexWriter.h"

using namespace boost;

const unsigned int IndexWriter::INDEX_WRITER_VERSION = 1;

IndexWriter::IndexWriter(const path &indexFile) : IndexProcessor(indexFile) {}

boost::shared_ptr<IndexWriter> IndexWriter::create(const path &indexFile) {
    auto iw = boost::shared_ptr<IndexWriter>(new IndexWriter(indexFile));
    bool result = iw->open();

    if (!result)
        return boost::shared_ptr<IndexWriter>(nullptr);
    return iw;
}

bool IndexWriter::open() {
    if (!lockForWriting())
        return false;
    if (exists(indexFile)) {
        unlock();
        return false;
    }
    this->outputStream = make_shared<ofstream>(indexFile);
    *this->outputStream << "";

    return exists(indexFile);
}

bool IndexWriter::writeIndexHeader(boost::shared_ptr<IndexHeader> header) {
    if (this->headerWasWritten)
        return false;

    outputStream->write((char *) header.get(), sizeof(IndexHeader));

    this->headerWasWritten = true;

    return true;
}

bool IndexWriter::writeIndexEntry(boost::shared_ptr<IndexEntry> entry) {
    if (!this->headerWasWritten)
        return false;

    outputStream->write((char *) entry.get(), sizeof(IndexEntry));

    return true;
}

void IndexWriter::flush() {
    this->outputStream->flush();
}
