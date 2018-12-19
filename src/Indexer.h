/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_INDEXER_H
#define FASTQINDEX_INDEXER_H

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

class Indexer {

private:
    path fastq;

    path index;

public:

    Indexer(const path &fastq, const path &index);

    bool createIndex();

};


#endif //FASTQINDEX_INDEXER_H
