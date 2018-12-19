/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#include "Indexer.h"

Indexer::Indexer(const path &fastq, const path &index) : fastq(fastq), index(index) {}

bool Indexer::createIndex() {}