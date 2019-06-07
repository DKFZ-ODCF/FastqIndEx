/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "IndexReader.h"
#include "IndexWriter.h"
#include <experimental/filesystem>
#include <fstream>

using namespace std;
using std::experimental::filesystem::path;


IndexReader::IndexReader(const path &indexFile) : IndexProcessor(indexFile), inputStream(nullptr) {
}

IndexReader::~IndexReader() {
    if (inputStream) {
        if (inputStream->good())
            inputStream->close();
        delete inputStream;
    }
}

bool IndexReader::tryOpenAndReadHeader() {

    if (readerIsOpen)
        return true;

    if (!exists(indexFile)) {
        addErrorMessage("The index file does not exist.");
        return false;
    }
    bool gotLock = openWithReadLock();
    if (!gotLock) {
        addErrorMessage("Could not get a lock for the index file.");
        return false;
    }

    uintmax_t fileSize = file_size(indexFile);

    size_t headerSize = sizeof(IndexHeader);
    if (headerSize > fileSize) {
        addErrorMessage("The index file is too small and cannot be read.");
        this->unlock();
        return false;
    }

    /**
     * Open the stream and see if it is good.
     */
    this->inputStream = new ifstream(indexFile);
    if (!this->inputStream->good()) {
        addErrorMessage("There was an error while opening input stream for index file.");
        this->inputStream->close();
        this->unlock();
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
        addErrorMessage("Index version is not readable with this version of FastqIndEx.");
        this->inputStream->close();
        this->unlock();
        return false;
    }

    if (headerSize + sizeOfIndexEntry > fileSize) {
        addErrorMessage("Cannot read index file, it is too small.");
        this->inputStream->close();
        this->unlock();
        return false;
    }

    if (0 != (fileSize - headerSize) % sizeOfIndexEntry) {
        addErrorMessage("Cannot read index file, there is a mismatch between stored index version and content size.");
        this->inputStream->close();
        this->unlock();
        return false;
    }

    this->indicesLeft = (fileSize - headerSize) / sizeOfIndexEntry;
    this->indicesCount = indicesLeft;

    this->readerIsOpen = true;

    return readerIsOpen;
}

IndexHeader IndexReader::readIndexHeader() {
    IndexHeader header;
    inputStream->read((char *) &header, sizeof(IndexHeader));

    this->readHeader = header;
    headerWasRead = true;
    return header;
}

vector<shared_ptr<IndexEntry>> IndexReader::readIndexFile() {
    vector<shared_ptr<IndexEntry>> convertedLines;
    if (!tryOpenAndReadHeader()) {
        addErrorMessage("Could not read index file due to one or more errors during file open.");
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
        addErrorMessage("Could not read index file due to one or more errors during file open.");
        return shared_ptr<IndexEntry>(nullptr);
    }

    // Read in and convert a specific header version to an IndexEntry vector
    if (this->readHeader.indexWriterVersion == 1) {
        return readIndexEntryV1()->toIndexEntry();
    } // We do not need an else branch, a version range check is applied earlier.
}

shared_ptr<IndexEntryV1> IndexReader::readIndexEntryV1() {
    if (!readerIsOpen) {
        addErrorMessage("You have to open the IndexReader instance first with tryOpenAndReadHeader()");
        return shared_ptr<IndexEntryV1>(nullptr);
    }

    //Whyever, eof does not seem to work reliably, so use indicesLeft.
    if (inputStream->eof() || indicesLeft <= 0) {
        addErrorMessage("The stream is finished and no entries are left to read. Can't read a new entry.");
        return shared_ptr<IndexEntryV1>(nullptr);
    }

    auto entry = make_shared<IndexEntryV1>();
    inputStream->read((char *) entry.get(), sizeof(IndexEntryV1));
    indicesLeft--;

    return entry;
}


