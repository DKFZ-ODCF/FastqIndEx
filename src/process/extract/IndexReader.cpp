/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexReader.h"
#include "process/index/IndexWriter.h"
#include "process/io/Source.h"
#include <experimental/filesystem>
#include <fstream>

using namespace std;
using std::experimental::filesystem::path;


IndexReader::IndexReader(const shared_ptr<Source> &indexFile) {
    this->indexFile = indexFile;
}

IndexReader::~IndexReader() {
    this->indexFile->close();
}

bool IndexReader::tryOpenAndReadHeader() {

    if (readerIsOpen)
        return true;

    if (!indexFile->exists()) {
        addErrorMessage("Index file '", indexFile->toString(), "' does not exist.");
        return false;
    }
    bool gotLock = indexFile->openWithReadLock();
    if (!gotLock) {
        addErrorMessage("Could not get a lock for index file '", indexFile->toString(), "'.");
        return false;
    }

    int64_t fileSize = static_cast<int64_t>(indexFile->size());

    int64_t headerSize = static_cast<int64_t>(sizeof(IndexHeader));
    if (headerSize > fileSize) {
        addErrorMessage("Index file '", indexFile->toString(), "' is smaller than the minimum size (",
                        to_string(fileSize), " < ", to_string(headerSize), " Bytes) and cannot be read.");
        indexFile->close();
        return false;
    }

    /**
     * Open the stream and see if it is good.
     */
    if (this->indexFile->isBad()) {
        addErrorMessage("Could not open an input stream for index file '", indexFile->toString(), "'.");
        indexFile->close();
        return false;
    }

    this->readIndexHeader();

    /**
     * Which IndexReader / IndexEntry version must be used. Extract this from the header and go on.
     */
    uint sizeOfIndexEntry;
    if (this->readHeader.indexWriterVersion == 1) {
        sizeOfIndexEntry = sizeof(IndexEntryV1);
    } else {
        addErrorMessage("Index version '", to_string(this->readHeader.indexWriterVersion),
                        "' is not readable with this version of FastqIndEx (The maximum is ", to_string(
                        IndexWriter::INDEX_WRITER_VERSION), "). You need a newer version of FastqIndEx.");
        indexFile->close();
        return false;
    }

    if (this->readHeader.dictionariesAreCompressed) {
        if (fileSize <= headerSize) {
            addErrorMessage("Index file '", indexFile->toString(), "' is smaller than the minimum size (",
                            to_string(fileSize), " < ", to_string(headerSize), " Bytes) and cannot be read.");
            indexFile->close();
            return false;
        }
    } else if (headerSize + sizeOfIndexEntry > fileSize) {
        addErrorMessage("Index file '", indexFile->toString(), "' is smaller than the minimum size (",
                        to_string(fileSize), " < ", to_string(headerSize + sizeOfIndexEntry),
                        " Bytes) and cannot be read.");
        indexFile->close();
        return false;
    }

    if (this->readHeader.dictionariesAreCompressed) {
        // We could only check, if there is at least one entry.
    } else if (0 != (fileSize - headerSize) % sizeOfIndexEntry) {
        addErrorMessage("Index file '", indexFile->toString(),
                        "' shows a mismatch mismatch between the stored index '",
                        to_string(this->readHeader.indexWriterVersion),
                        "' version and content size.");
        indexFile->close();
        return false;
    }

    if (this->readHeader.numberOfEntries > 0) { // Easy case, it is stored.
        this->indicesLeft = this->readHeader.numberOfEntries;
    } else if (!this->readHeader.dictionariesAreCompressed) {
        this->indicesLeft = (fileSize - headerSize) / sizeOfIndexEntry;
    }

    if (this->indicesLeft == 0) {
        addErrorMessage("Could not determine the amount of entries in index file '", indexFile->toString(), "'.");
        indexFile->close();
        return false;
    }

    this->indicesCount = indicesLeft;

    this->readerIsOpen = true;

    return readerIsOpen;
}

IndexHeader IndexReader::readIndexHeader() {
    IndexHeader header;
    indexFile->read(reinterpret_cast<Bytef *>(&header), sizeof(IndexHeader));

    this->readHeader = header;
    headerWasRead = true;
    return header;
}

vector<shared_ptr<IndexEntry>> IndexReader::readIndexFile() {
    vector<shared_ptr<IndexEntry>> convertedLines;
    if (!tryOpenAndReadHeader()) {
        addErrorMessage("Could not read index file '", indexFile->toString(),
                        "' due to one or more errors during file open.");
        return convertedLines;
    }

    // Read in and convert a specific header version to an IndexEntry vector
    if (this->readHeader.indexWriterVersion == 1) {
        auto entries = readIndexFileV1();
        for (auto const &entry : entries) {
            convertedLines.emplace_back(entry->toIndexEntry());
        }
    } // We do not need an else branch, a version range check is applied earlier.

    return convertedLines;
}

vector<shared_ptr<IndexEntryV1>> IndexReader::readIndexFileV1() {
    vector<shared_ptr<IndexEntryV1>> convertedLines;
    // Initialize an empty index entry to make following steps a bit easier.
    while (indicesLeft > 0) {
        auto indexEntry = readIndexEntryV1();
        if (indexEntry) {
            convertedLines.emplace_back(indexEntry);
        }
    }
    return convertedLines;
}

shared_ptr<IndexEntry> IndexReader::readIndexEntry() {
    if (!tryOpenAndReadHeader()) {
        addErrorMessage("Could not read index file '", indexFile->toString(),
                        "' due to one or more errors during file open.");
        return shared_ptr<IndexEntry>(nullptr);
    }

    // Read in and convert a specific header version to an IndexEntry vector
    if (this->readHeader.indexWriterVersion == 1) {
        return readIndexEntryV1()->toIndexEntry();
    } // We do not need an else branch, a version range check is applied earlier.

    // To avoid clang-tidy from complaining, we return an empty shared_ptr. And we also add an error message just to be
    // sure.
    addErrorMessage("BUG: It must not be possible to reach this code part in IndexReader::readIndexEntry()");
    return shared_ptr<IndexEntry>(nullptr);
}

shared_ptr<IndexEntryV1> IndexReader::readIndexEntryV1() {
    if (!readerIsOpen) {
        addErrorMessage("BUG: You have to open the IndexReader instance first with tryOpenAndReadHeader()");
        return shared_ptr<IndexEntryV1>(nullptr);
    }

    //Whyever, eof does not seem to work reliably, so use indicesLeft.
    if (indexFile->eof() || indicesLeft <= 0) {
        addErrorMessage("The stream is finished and no entries are left to read. Can't read a new entry.");
        return shared_ptr<IndexEntryV1>(nullptr);
    }

    auto entry = make_shared<IndexEntryV1>();
    int headerSize = sizeof(IndexEntryV1) - sizeof(entry->dictionary);
    indexFile->read(reinterpret_cast<Bytef *>(entry.get()), headerSize);
    if (entry->compressedDictionarySize == 0) { // No compression
        indexFile->read(reinterpret_cast<Bytef *>(entry.get()) + headerSize, sizeof(entry->dictionary));
    } else {
        // Set the dictionary to 0 first, so we won't have any issues with memory garbage.
        memset(reinterpret_cast<char *>(entry->dictionary), 0, WINDOW_SIZE);
        indexFile->read(reinterpret_cast<Bytef *>(entry.get()) + headerSize, entry->compressedDictionarySize);
    }
    indicesLeft--;

    return entry;
}