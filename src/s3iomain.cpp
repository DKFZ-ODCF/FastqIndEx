/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */

#include "process/io/s3/FQIS3Client.h"
#include "process/io/s3/S3Service.h"
#include "common/CommonStructsAndConstants.h"
#include <experimental/filesystem>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std::experimental::filesystem;

/**
 * Helper application to download files from s3 in a separate process Why? Because we did not find a way to
 * abort / cancel / stop a running S3 asynchronous get request. The best attempt still resulted in a broken pipe which
 * is hardly a trustworthy error message for someone using the S3 functionality.
 */

tuple<FILE *, string> openProcOrUserFile(const pid_t &pid, const string &suffix) {
    char procfile[BUFSIZ];
    if (exists("/proc"))
        sprintf(procfile, "/proc/%d/%s", pid, suffix.c_str());
    else if (exists("/user"))
        sprintf(procfile, "/user/%d/%s", pid, suffix.c_str());
    else
        return {nullptr, ""};

    FILE *f = fopen(procfile, "r");
    if (!f) return {f, ""};
    return {f, string(procfile)};
}

/**
 * Get the name of the current process.
 *
 * @see https://gist.github.com/fclairamb/a16a4237c46440bdb172
 */
tuple<bool, string> getProcessName(const pid_t pid) {
    auto[filePointer, file] = openProcOrUserFile(pid, "cmdline");
    if (!filePointer) return {false, ""};

    char buffer[WINDOW_SIZE]{0};
    auto readBytes = fread(buffer, sizeof(char), sizeof(buffer), filePointer);
    if (readBytes > 0 && '\n' == buffer[readBytes - 1])
        buffer[readBytes - 1] = '\0';
    fclose(filePointer);
    return {true, string(buffer)};
}

/**
 * Get the pid of this applications parent process.
 *
 * @see https://gist.github.com/fclairamb/a16a4237c46440bdb172
 */
tuple<bool, pid_t> getProcessParentID(const pid_t pid) {
    auto[filePointer, file] = openProcOrUserFile(pid, "stat");
    if (!filePointer) return {false, 0};

    char buffer[WINDOW_SIZE]{0};
    auto readBytes = fread(buffer, sizeof(char), sizeof(buffer), filePointer);

    if (readBytes > 0) {
        // See: http://man7.org/linux/man-pages/man5/proc.5.html section /proc/[pid]/stat
        strtok(buffer, " ");           // (1) pid  %d
        strtok(nullptr, " ");       // (2) comm  %s
        strtok(nullptr, " ");       // (3) state  %c
        char *s_ppid = strtok(nullptr, " "); // (4) ppid  %d

        return {true, atoi(s_ppid)};
    }
    fclose(filePointer);
    return {false, 0};
}

/**
 * Returns the parent path of the this binary.
 */
tuple<bool, path> getParentPath() {
    int myPid = getpid();
    auto[processParentIDValid, processParentID] = getProcessParentID(myPid);
    if(!processParentIDValid) {
        std::cerr << "Could not get process id for parent process.\n";
        return {false, ""};
    }

    auto[processNameValid, processName] = getProcessName(processParentID);
    if(!processNameValid) {
        std::cerr << "Could not get the name of the process.\n";
        return {false, ""};
    }

    return {true, processName};
}

/**
 * The main function will 6 parameters (+1 for the application path in parameter 0).
 * The application is not meant for standalone usage and does not feature help messages.
 * @param argv  The argument list for the program. The necessary values are:
 *              [0] - Application path / set by the system (actually not necessary)
 *              [1] - The fifo or file to which the GetObject request will write.
 *              [2] - The S3 object like in s3://bucket/object - It is the same as for the fastqindex binary.
 *              [3] - The S3 configuration sections or "default"
 *              [4] - The S3 configuration file or (IMPORTANT) " " <- See the whitespace inside double ticks?
 *              [5] - The S3 credentials file or (ALSO HERE) " " <- Again the whitespace surrounded by " " double ticks!
 *              [6] - The starting position.
 * @return Some value, most likely an error value as we are not able to properly abort S3 GetObject
 */
int main(int argc, char **argv) {
    auto [parentPathRetrieved, parentPath] = getParentPath();
    if(!parentPathRetrieved) {
        std::cerr << "Could not get the parent processes path.\n";
        return -1;
    }
    if (parentPath != "fastqindex") {
        std::cerr << "The FastqIndEx S3 helper binary was not started by FastqIndEx itself! "
                  << "Only do this for tests.\n";
    }

    if (argc != 7)
        return 1;

    path fifo = argv[1];
    string s3Object = argv[2];
    S3ServiceOptions s3ServiceOptions(argv[5], argv[4], argv[3]);
    int64_t readStart = stoll(argv[6]);

    FQIS3Client fqiS3Client(s3Object, s3ServiceOptions);

    auto s3 = S3Service::getInstance();

    // Create and start asynchronous request
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(fqiS3Client.getBucketName().c_str());
    object_request.SetKey(fqiS3Client.getObjectName().c_str());

    auto[success, size] = fqiS3Client.getObjectSize();

    if (!success) {
        cerr << "Requesting size of '" << fqiS3Client.getS3Path() << "' failed. Will waitForFinish now.\n";
        return -2;
    }

    if (readStart != 0) {
        auto rangeString = "bytes=" + to_string(readStart) + "-" + to_string(size);
        object_request.SetRange(rangeString.c_str());
    }
    string object = fqiS3Client.getObjectName();
    object_request.SetResponseStreamFactory([&]() {
        auto stream = Aws::New<FStream>(
                fqiS3Client.getObjectName().c_str(),
                fifo.c_str(),
                std::ios_base::out | std::ios_base::binary);
        return stream;
    });

    auto context = Aws::MakeShared<Aws::Client::AsyncCallerContext>("GetObjectAllocationTag");
    context->SetUUID(fqiS3Client.getObjectName().c_str());
    auto client = s3->getClient();

    auto outcome = s3->getClient()->GetObject(object_request);
    if (outcome.IsSuccess())
        return 0;

    cerr << "Something went wrong during a download from S3 for '" << fqiS3Client.getS3Path() << "'.";
    return -3;
}
