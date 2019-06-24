//
// Created by heinold on 25.06.19.
//

#ifndef FASTQINDEX_MODECLIPARSER_H
#define FASTQINDEX_MODECLIPARSER_H


#include "../runners/Runner.h"
#include <tclap/CmdLine.h>

using namespace TCLAP;

class ModeCLIParser {

public:

    static path argumentToPath(ValueArg<string> &cliArg);

    virtual Runner *parse(int argc, const char **argv) = 0;
};


#endif //FASTQINDEX_MODECLIPARSER_H
