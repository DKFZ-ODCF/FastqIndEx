/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
 */

#ifndef FASTQINDEX_RUNNER_H
#define FASTQINDEX_RUNNER_H

#include <string>
#include <vector>

using namespace std;

class Runner {
protected:
    Runner() = default;

    vector<string> errorMessages;

public:

    vector<string> getErrorMessages() { return errorMessages; }

    /**
     * Can return an exit between 0 and 255
     * @return
     */
    virtual unsigned char run() {};

    virtual bool checkPremises() { return true; };

    /**
     * Actually for debugging.
     * @return
     */
    virtual bool isShowStopper() { return false; };

    /**
     * Actually for debugging.
     * @return
     */
    virtual bool isIndexer() { return false; };

    /**
     * Actually for debugging.
     * @return
     */
    virtual bool isExtractor() { return false; };
};

class ShowStopperRunner : public Runner {
public :
    ShowStopperRunner() = default;

    unsigned char run() override;

    bool isShowStopper() override { return true; }

};





#endif //FASTQINDEX_RUNNER_H
