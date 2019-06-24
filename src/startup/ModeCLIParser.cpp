//
// Created by heinold on 25.06.19.
//

#include <tclap/CmdLine.h>
#include <cstring>
#include "Starter.h"
#include "IndexModeCLIParser.h"
#include "ExtractModeCLIParser.h"
#include "../process/io/StreamInputSource.h"
#include "../process/io/PathInputSource.h"
#include "../process/io/InputSource.h"
#include "../runners/IndexerRunner.h"
#include "../runners/ExtractorRunner.h"
#include "ModeCLIParser.h"

path ModeCLIParser::argumentToPath(ValueArg<string> &cliArg) {
    path _path;
    if (cliArg.getValue() == "-")
        _path = path("-");
    else
        _path = path(cliArg.getValue());
    return _path;
}