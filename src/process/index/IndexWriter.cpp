/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include <fstream>
#include <iostream>
#include "IndexWriter.h"
#include "../../common/ErrorAccumulator.h"

const unsigned int IndexWriter::INDEX_WRITER_VERSION = 1;


IndexWriter::IndexWriter(const path &indexFile, bool forceOverwrite, bool compressionIsActive)
        : IndexProcessor(indexFile) {
    this->forceOverwrite = forceOverwrite;
    this->compressionIsActive = compressionIsActive;
}

IndexWriter::~IndexWriter() {
//    lock_guard<mutex> lock(iwMutex);
    finalize();
}

bool IndexWriter::tryOpen() {
    lock_guard<mutex> lock(iwMutex);
    if (writerIsOpen)
        return true;

    if (!forceOverwrite && exists(indexFile)) {
        addErrorMessage("The index file cannot be overwritten: " + indexFile.string());
        unlock();
        return false;
    }
    if (!openWithWriteLock()) {
        addErrorMessage("Could not get a lock for index file: " + indexFile.string());
        return false;
    }
    auto o = ofstream();
    this->outputStream.open(indexFile, ios_base::out | ios_base::in | ios_base::binary);
    this->outputStream.write("", 0);

    if (!exists(indexFile)) {
        addErrorMessage("Could not create index file: " + indexFile.string());
        unlock();
        return false;
    }

    writerIsOpen = true;

    return true;
}

bool IndexWriter::writeIndexHeader(const shared_ptr<IndexHeader> &header) {
    lock_guard<mutex> lock(iwMutex);
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

    outputStream.write((char *) header.get(), sizeof(IndexHeader));

    this->headerWasWritten = true;

    return true;
}

bool IndexWriter::writeIndexEntry(const shared_ptr<IndexEntryV1> &entry) {
    lock_guard<mutex> lock(iwMutex);
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

    numberOfWrittenEntries++;

    if (entry->compressedDictionarySize == 0) // No compression
        outputStream.write((char *) entry.get(), sizeof(IndexEntryV1));
    else {
        int headerSize = sizeof(IndexEntryV1) - sizeof(entry->dictionary);
        outputStream.write((char *) entry.get(), headerSize);
        outputStream.write((char *) entry.get() + headerSize, entry->compressedDictionarySize);
    }

    return true;
}

void IndexWriter::flush() {
    lock_guard<mutex> lock(iwMutex);
    if (this->outputStream) {
        this->outputStream.flush();
    }
}

void IndexWriter::finalize() {
    lock_guard<mutex> lock(iwMutex);
    if (this->outputStream.is_open()) {
        this->writerIsOpen = false;
        // Without flush, the file size was 0, even after closing the stream.
        // Important: I do not work with reentrant locks here! Don't call the flush() method above or you'll encounter
        // a nice deadlock.
        outputStream.flush();
        outputStream.seekp(16, ios_base::beg);
        outputStream.write((const char*)&numberOfWrittenEntries, 8);
        outputStream.seekp(24, ios_base::beg);
        outputStream.write((const char*)&numberOfLinesInFile, 8);
        outputStream.flush();
        this->outputStream.close();
    }
}