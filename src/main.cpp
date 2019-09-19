/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "main.h"
#include "startup/Starter.h"


int main(int argc, const char *argv[]) {
    // Store the application binary path.
    path fqiBinaryPath = IOHelper::getApplicationPath();
    path s3HelperBinary = IOHelper::fullPath(fqiBinaryPath.parent_path().string() + "/S3IOHelperForFastqIndEx");
    strcpy(FQI_BINARY, fqiBinaryPath.string().c_str());
    strcpy(S3HELPER_BINARY, s3HelperBinary.string().c_str());
    ErrorAccumulator::always("FQI binary path:       '", fqiBinaryPath, "'");
    ErrorAccumulator::always("S3 helper binary path: '", s3HelperBinary, "'");

    Starter starter;
    auto runner = starter.createRunner(argc, argv);

    int exitCode = 0;
    if (!runner->fulfillsPremises()) {
        cerr << "There were errors preventing FastqIndEx to start:\n";
        exitCode = 1;
    } else {
        exitCode = runner->run();
    }
    if (exitCode != 0)
        for (auto const &message : runner->getErrorMessages()) {
            cerr << "\t" << message << "\n";
        }

    return exitCode;
}