/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXWRITER_H
#define FASTQINDEX_INDEXWRITER_H

#include "common/CommonStructsAndConstants.h"
#include "process/io/locks/PathLockHandler.h"
#include "process/io/Sink.h"
#include <experimental/filesystem>
#include <fstream>

using namespace std;
using std::experimental::filesystem::path;


class IndexWriter : public ErrorAccumulator {
private:

    mutex iwMutex;

    shared_ptr<Sink> indexFile;

    /**
     * You are not allowed to write an index twice.
     * You are also not allowed to write an entry before the index.
     */
    bool headerWasWritten = false;

    bool writerIsOpen = false;

    bool forceOverwrite = false;

    bool compressionIsActive = true;

    u_int64_t numberOfWrittenEntries{0};

    /**
     * Must be set after the the index process via setNumberOfLinesInFile
     */
    u_int64_t numberOfLinesInFile{0};

//    std::fstream fStream = std::fstream();

public:

    static const unsigned int INDEX_WRITER_VERSION;

    explicit IndexWriter(const shared_ptr<Sink> &indexFile, bool forceOverwrite = false,
                         bool compressionIsActive = true);

    virtual ~IndexWriter();

    void setNumberOfLinesInFile(u_int64_t numberOfLinesInFile) {
        this->numberOfLinesInFile = numberOfLinesInFile;
    }

    bool tryOpen();

    bool hasLock() {
        if (indexFile.get())
            return indexFile->hasLock();
        return false;
    }

    bool writeIndexHeader(const shared_ptr<IndexHeader> &header);

    bool writeIndexEntry(const shared_ptr<IndexEntryV1> &entry);

    void flush();

    bool close();

    void finalize();

    vector<string> getErrorMessages() override;
};


#endif //FASTQINDEX_INDEXWRITER_H
