/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include <boost/make_shared.hpp>
#include <boost/filesystem/fstream.hpp>
#include "IndexWriter.h"
#include "ErrorAccumulator.h"

using namespace boost;

const unsigned int IndexWriter::INDEX_WRITER_VERSION = 1;


IndexWriter::IndexWriter(const path &indexFile) : IndexProcessor(indexFile) {
}

bool IndexWriter::tryOpen() {
    if (writerIsOpen)
        return true;

    if (!lockForWriting()) {
        addErrorMessage("Could not get a lock for index file: " + indexFile.string());
        return false;
    }
    if (exists(indexFile)) {
        addErrorMessage("The index file cannot be overwritten: " + indexFile.string());
        unlock();
        return false;
    }
    this->outputStream = make_shared<boost::filesystem::ofstream>(indexFile);
    *this->outputStream << "";

    if (!exists(indexFile)) {
        addErrorMessage("Could not create index file: " + indexFile.string());
        unlock();
        return false;
    }

    writerIsOpen = true;

    return true;
}

bool IndexWriter::writeIndexHeader(boost::shared_ptr<IndexHeader> header) {
    if (!this->writerIsOpen) {
        // Throw assertion errors? Would actually be better right?
        addErrorMessage("Could not write header to index file, writer is not open.");
        return false;
    }

    if (this->headerWasWritten) {
        // Throw assertion errors? Would actually be better right?
        addErrorMessage("It is not allowed to write the index header more than once.");
        return false;
    }

    outputStream->write((char *) header.get(), sizeof(IndexHeader));

    this->headerWasWritten = true;

    return true;
}

bool IndexWriter::writeIndexEntry(boost::shared_ptr<IndexEntryV1> entry) {
    if (!this->writerIsOpen) {
        // Throw assertion errors? Would actually be better right?
        addErrorMessage("Could not write index entry to index file, writer is not open.");
        return false;
    }

    if (!this->headerWasWritten) {
        // Throw assertion errors? Would actually be better right?
        addErrorMessage("The index header must be written to the index file before the entries.");
        return false;
    }

    outputStream->write((char *) entry.get(), sizeof(IndexEntryV1));

    return true;
}

void IndexWriter::flush() {
    if (this->outputStream) {
        this->outputStream->flush();
    }
}

IndexWriter::~IndexWriter() {
    if (this->outputStream) {
        flush(); // Without flush, the file size was 0, even after closing the stream.
        this->outputStream->close();
    }
}
