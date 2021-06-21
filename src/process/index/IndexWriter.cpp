/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "common/ErrorAccumulator.h"
#include "IndexWriter.h"
#include <iostream>

const unsigned int IndexWriter::INDEX_WRITER_VERSION = 1;

IndexWriter::IndexWriter(const shared_ptr<Sink> &indexFile, bool forceOverwrite, bool compressionIsActive) {
    this->indexFile = indexFile;
    this->forceOverwrite = forceOverwrite;
    this->compressionIsActive = compressionIsActive;
}

IndexWriter::~IndexWriter() {
    finalize();
}

bool IndexWriter::tryOpen() {
    lock_guard<mutex> lock(iwMutex);
    if (writerIsOpen)
        return true;

    if (!indexFile->fulfillsPremises()) {
        indexFile->unlock();
        return false;
    }

    if (!indexFile->openWithWriteLock()) {
        addErrorMessage("Could not get a lock for index file: '" + indexFile->toString(), "'.");
        return false;
    }

    indexFile->write("", 0);

    if (!indexFile->exists()) {
        addErrorMessage("Could not create index file '" + indexFile->toString() + "'.");
        indexFile->unlock();
        return false;
    }

    writerIsOpen = true;

    return true;
}

bool IndexWriter::writeIndexHeader(const shared_ptr<IndexHeader> &header) {
    lock_guard<mutex> lock(iwMutex);
    if (!this->writerIsOpen) {
        addErrorMessage("BUG: Could not write header to index file '" + indexFile->toString() + "', writer is closed.");
        return false;
    }

    if (this->headerWasWritten) {
        addErrorMessage("BUG: It is not allowed to write the index header more than once.");
        return false;
    }

    indexFile->write(reinterpret_cast<char *>( header.get()), sizeof(IndexHeader));

    this->headerWasWritten = true;

    return true;
}

bool IndexWriter::writeIndexEntry(const shared_ptr<IndexEntryV1> &entry) {
    lock_guard<mutex> lock(iwMutex);
    if (!this->writerIsOpen) {
        addErrorMessage(
                "BUG: Could not write index entry to index file '" + indexFile->toString() + "', writer is closed.");
        return false;
    }

    if (!this->headerWasWritten) {
        addErrorMessage("BUG: The index header must be written to the index file before an entry can be written.");
        return false;
    }

    numberOfWrittenEntries++;

    if (entry->compressedDictionarySize == 0) // No compression
        indexFile->write(reinterpret_cast<char *>( entry.get()), sizeof(IndexEntryV1));
    else {
        int headerSize = sizeof(IndexEntryV1) - sizeof(entry->dictionary);
        indexFile->write(reinterpret_cast<char *>(entry.get()), headerSize);
        indexFile->write(reinterpret_cast<char *>( entry.get()) + headerSize, entry->compressedDictionarySize);
    }

    return true;
}

void IndexWriter::flush() {
    lock_guard<mutex> lock(iwMutex);
    this->indexFile->flush();
}

void IndexWriter::finalize() {
    lock_guard<mutex> lock(iwMutex);
    if (this->indexFile->isOpen()) {
        this->writerIsOpen = false;
        // Without flush, the file size was 0, even after closing the stream.
        // Important: I do not work with reentrant locks! Don't call the this->flush() or you'll encounter a deadlock.
        indexFile->flush();
        indexFile->seek(16, true);
        indexFile->write(reinterpret_cast<const char *>( &numberOfWrittenEntries), 8);
        indexFile->seek(24, true);
        indexFile->write(reinterpret_cast<const char *>(&numberOfLinesInFile), 8);
        indexFile->flush();
        this->indexFile->close();
    }
}

vector<string> IndexWriter::getErrorMessages() {
    auto l = ErrorAccumulator::getErrorMessages();
    auto r = indexFile->getErrorMessages();
    return concatenateVectors(l, r);
}
