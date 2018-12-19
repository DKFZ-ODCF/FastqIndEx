/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_COMMONSTRUCTS_H
#define FASTQINDEX_COMMONSTRUCTS_H

#include <cstring>

struct IndexHeader {
    unsigned int binary_version;
    unsigned int reserved_0 = 0;
    unsigned long reserved_1 = 0;
    unsigned long reserved_2 = 0;
    unsigned long reserved_3 = 0;
    unsigned int magic_no;

    explicit IndexHeader(unsigned int binary_version) {
        char mno[] = {1, 2, 3, 4};
        magic_no = *(int *) &mno;
        this->binary_version = binary_version;
    }
};

struct IndexEntry {
    unsigned int entry_no = 0;
    unsigned long offset = 0;
    unsigned char bits = 0;
    unsigned long starting_line_in_entry = 0;
    unsigned char line_starts_at_pos_0 = 0;

    IndexEntry(unsigned int entry_no, unsigned long offset, unsigned char bits, unsigned long starting_line_in_entry,
               unsigned char line_starts_at_pos_0) :
            entry_no(entry_no),
            offset(offset),
            bits(bits),
            starting_line_in_entry(starting_line_in_entry),
            line_starts_at_pos_0(line_starts_at_pos_0) {}

    IndexEntry() {
        memset(this, 0, sizeof(IndexEntry));
    }
};

#endif //FASTQINDEX_COMMONSTRUCTS_H
