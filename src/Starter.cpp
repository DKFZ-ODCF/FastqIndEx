/**
 * Copyright (c) 2018 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "Starter.h"
#include "IndexerRunner.h"
#include "ExtractorRunner.h"
#include <boost/program_options.hpp>
#include <cstring>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace boost::program_options;

Starter::Starter() {
    assembleCLIOptions();
}

Starter *Starter::instance = nullptr;

Starter *Starter::getInstance() {
    boost::mutex lock;
    lock.lock();
    if (!Starter::instance) Starter::instance = new Starter();
    lock.unlock();
    return Starter::instance;
}

/**
 * Effectively checks parameter count and file existence and accessibility
 * @param argc parameter count
 * @param argv parameter array
 */
void Starter::assembleCLIOptions() {
    options_description index("Options for index");
    index.add_options()
            (INDEX_MODE, "Index a fastq file.")
            (INDEXFILE_PARAMETER, value<string>(), "The index file which shall be generated.");

    options_description extract("Options for extract");
    extract.add_options()
            (EXTRACTION_MODE, "Extract lines from an indexed fastq file")
//            (INDEXFILE_PARAMETER, value<string>(), "The index file which shall be used.")
            (STARTLINE_PARAMETER, value<ulong>(), "The starting line.")
            (NOOFREADS_PARAMETER, value<ulong>(), "The number of reads which shall be extracted.");

//    options_description hidden("Hidden options which should not be printed.");
    hidden.add_options()
            (FASTQFILE_PARAMETER, value<string>(), "The fastq file which will be used.");

    posCliOptions.add(FASTQFILE_PARAMETER, 1);
    cliOptions.add(index);
    cliOptions.add(extract);
}

options_description *Starter::getCLIOptions() {
    return &cliOptions;
}

boost::shared_ptr<Runner> Starter::createRunner(int argc, const char *argv[]) {
    variables_map vm;
    options_description allOptions;
    allOptions.add(cliOptions).add(hidden);
    store(command_line_parser(argc, argv).options(allOptions).positional(posCliOptions).run(), vm);

    if (vm.count(FASTQFILE_PARAMETER)) {
        string fastqfile = vm[FASTQFILE_PARAMETER].as<string>();
        string indexfile;
        if (vm.count(INDEXFILE_PARAMETER)) {
            indexfile = vm[INDEXFILE_PARAMETER].as<string>();
        } else {
            indexfile = fastqfile + ".idx";
        }
        if (vm.count(INDEX_MODE)) {
            // Check everything and return a IndexerRunner object
            return boost::shared_ptr<Runner>(new IndexerRunner(fastqfile, indexfile));
        } else if (vm.count(EXTRACTION_MODE) == 1) {
            ulong startingRead;
            ulong readCount;
            startingRead = vm[STARTLINE_PARAMETER].as<ulong>();
            readCount = vm[NOOFREADS_PARAMETER].as<ulong>();
            return boost::shared_ptr<Runner>(new ExtractorRunner(fastqfile, indexfile, startingRead, readCount));
        }
    }

    return boost::shared_ptr<Runner>(new PrintCLIOptionsRunner());
}
