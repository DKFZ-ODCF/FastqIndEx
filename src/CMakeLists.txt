#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
#add_definitions(-DUSE_IMPORT_EXPORT)
#find_package(AWSSDK REQUIRED COMPONENTS s3)

# Main application
add_executable(fastqindex main.cpp main.h)

# Helper application for S3
# Refer to s3iomain for more info.
add_executable(fastqindexs3iohelper s3iomain.cpp)

include_directories(
        ${CMAKE_SOURCE_DIR}/src
)

# Glob together files in src and add compilation flags for debug. This is tod isable additional compiler checks for
# external headers and sources. We can't change them so why check them.
# We still will add all files manually later. Why? Because cmake won't necessarily find additional files and folders
# automatically!
#file(GLOB_RECURSE ALL_SOURCES ${PROJECT_SOURCE_DIR} *.cpp *.h)
#message(${ALL_SOURCES})
#set_source_files_properties(${ALL_SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Wreturn-type -pedantic -ansi -Winit-self -Wmissing-declarations -Wextra -Winit-self -Wold-style-cast -Woverloaded-virtual -Wuninitialized")
#set_source_files_properties(zlib.h PROPERTIES COMPILE_FLAGS -Wnoold-style-cast++)

add_library(
        fastqindexlib
        common/CommonStructsAndConstants.cpp common/CommonStructsAndConstants.h
        common/ErrorAccumulator.cpp common/ErrorAccumulator.h
        common/ErrorMessages.cpp common/ErrorMessages.h
        common/IOHelper.cpp common/IOHelper.h
        common/Result.h
        common/StringHelper.cpp common/StringHelper.h
        process/base/BaseIndexEntry.h
        process/base/IndexHeader.cpp process/base/IndexHeader.h
        process/base/IndexEntry.cpp process/base/IndexEntry.h
        process/base/IndexEntryV1.h
        process/base/ZLibBasedFASTQProcessorBaseClass.cpp process/base/ZLibBasedFASTQProcessorBaseClass.h
        process/extract/Extractor.cpp process/extract/Extractor.h
        process/extract/IndexReader.cpp process/extract/IndexReader.h
        process/index/IndexEntryStorageDecisionStrategy.h
        process/index/Indexer.cpp process/index/Indexer.h
        process/index/IndexWriter.cpp process/index/IndexWriter.h
        process/io/s3/FQIS3Client.h
        process/io/s3/S3Config.cpp process/io/s3/S3Config.h
        process/io/s3/S3GetObjectProcessWrapper.cpp process/io/s3/S3GetObjectProcessWrapper.h
        process/io/s3/S3Service.cpp process/io/s3/S3Service.h
        process/io/s3/S3ServiceOptions.h
        process/io/s3/S3Sink.h
        process/io/s3/S3Source.cpp process/io/s3/S3Source.h
        process/io/locks/LockHandler.h
        process/io/locks/FileLockHandler.cpp process/io/locks/FileLockHandler.h
        process/io/locks/S3LockHandler.h
        process/io/IOBase.h
        process/io/FileSink.cpp process/io/FileSink.h
        process/io/FileSource.cpp process/io/FileSource.h
        process/io/Sink.h
        process/io/Source.h
        process/io/ConsoleSink.h
        process/io/StreamSource.cpp process/io/StreamSource.h
        runners/ActualRunner.h
        runners/ExtractorRunner.cpp runners/ExtractorRunner.h
        runners/IndexerRunner.cpp runners/IndexerRunner.h
        runners/IndexReadingRunner.h
        runners/IndexStatsRunner.cpp runners/IndexStatsRunner.h
        runners/IndexWritingRunner.h
        runners/Runner.cpp runners/Runner.h
        runners/PrintCLIOptions.cpp runners/PrintCLIOptions.h
        startup/ExtractModeCLIParser.cpp startup/ExtractModeCLIParser.h
        startup/IndexModeCLIParser.cpp startup/IndexModeCLIParser.h
        startup/IndexStatsModeCLIParser.h
        startup/ModeCLIParser.cpp startup/ModeCLIParser.h
        startup/Starter.cpp startup/Starter.h
)

target_link_libraries(
        fastqindexlib
        LINK_PUBLIC
        stdc++fs
        aws-cpp-sdk-s3
        aws-cpp-sdk-core
        ${CMAKE_THREAD_LIBS_INIT}
        ${ZLIB_LIBRARY}
)

target_link_libraries(
        fastqindex
        fastqindexlib
)

target_link_libraries(
        fastqindexs3iohelper
        fastqindexlib
)

set_target_properties(fastqindex PROPERTIES COMPILE_FLAGS "-Wreturn-type -pedantic -ansi -Winit-self -Wextra -Wold-style-cast -Woverloaded-virtual -Wuninitialized")
