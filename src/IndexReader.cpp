/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <boost/make_shared.hpp>
#include <boost/filesystem/fstream.hpp>
#include "IndexReader.h"
#include "IndexWriter.h"

using namespace boost;

boost::shared_ptr<IndexReader> IndexReader::create(const path &indexFile) {
    auto indexReader = new IndexReader(indexFile);
    auto ir = boost::shared_ptr<IndexReader>(indexReader);
    bool result = indexReader->open();

    if (!result)
        return boost::shared_ptr<IndexReader>(nullptr);
    return ir;
}

IndexReader::IndexReader(const path &indexFile) : IndexProcessor(indexFile) {
    this->inputStream = nullptr;
    this->indicesLeft = 0;
}

bool IndexReader::open() {
    if (!exists(indexFile))
        return false;
    if (!lockForReading())
        return false;
    uintmax_t fileSize = file_size(indexFile);
    if (sizeof(IndexHeader) + sizeof(IndexEntryV1) > fileSize)
        return false;
    if (0 != (fileSize - sizeof(IndexHeader)) % sizeof(IndexEntryV1))
        return false;

    this->indicesLeft = (fileSize - sizeof(IndexHeader)) / sizeof(IndexEntryV1);
    
    this->inputStream = new boost::filesystem::ifstream(indexFile);
    return this->inputStream->good();
}

boost::shared_ptr<IndexHeader> IndexReader::readIndexHeader() {
    if (headerWasRead)
        return boost::shared_ptr<IndexHeader>(nullptr);

    auto header = make_shared<IndexHeader>(IndexWriter::INDEX_WRITER_VERSION, sizeof(IndexEntryV1));
    inputStream->read((char *) header.get(), sizeof(IndexHeader));

    headerWasRead = true;
    return header;
}

boost::shared_ptr<IndexEntryV1> IndexReader::readIndexEntry() {
    //Whyever, eof does not seem to work reliably, so use indicesLeft.
    if (!headerWasRead || inputStream->eof() || indicesLeft <= 0)
        return boost::shared_ptr<IndexEntryV1>(nullptr);

    auto entry = make_shared<IndexEntryV1>();
    inputStream->read((char *) entry.get(), sizeof(IndexEntryV1));
    indicesLeft--;
    return entry;
}

IndexReader::~IndexReader() {
    if (inputStream) {
        if (inputStream->good())
            inputStream->close();
        delete inputStream;
    }
}

