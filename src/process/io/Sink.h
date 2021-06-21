/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_SINK_H
#define FASTQINDEX_SINK_H

#include "process/io/IOBase.h"

/**
 * Base class for various different types of output sinks:
 * - Path
 * - S3
 * -
 */
class Sink : public IOBase {
protected:

    bool forceOverwrite{false};

public:

    explicit Sink(bool forceOverwrite) : forceOverwrite(forceOverwrite) {

    }

    /**
     * Open, regardless if the output object was locked by another process or not.
     */

    virtual bool openWithWriteLock() { return true; };

    virtual void write(const char *message) = 0;

    virtual void write(const char *message, int len) = 0;

    virtual void write(const string &message) = 0;

    virtual void flush() = 0;

};

#endif //FASTQINDEX_SINK_H
