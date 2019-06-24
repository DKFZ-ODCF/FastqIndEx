#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
#add_definitions(-DUSE_IMPORT_EXPORT)
#find_package(AWSSDK REQUIRED COMPONENTS s3)
add_executable(fastqindex main.cpp main.h)

add_library(
        fastqindexlib
        runners/ActualRunner.cpp runners/ActualRunner.h
        common/CommonStructsAndConstants.cpp common/CommonStructsAndConstants.h
        common/ErrorAccumulator.cpp common/ErrorAccumulator.h
        common/ErrorMessages.cpp common/ErrorMessages.h
        process/extract/Extractor.cpp process/extract/Extractor.h
        runners/ExtractorRunner.cpp runners/ExtractorRunner.h
        runners/IndexerRunner.cpp runners/IndexerRunner.h
        process/index/Indexer.cpp process/index/Indexer.h
        process/extract/IndexReader.cpp process/extract/IndexReader.h
        process/index/IndexWriter.cpp process/index/IndexWriter.h
        process/base/IndexProcessor.cpp process/base/IndexProcessor.h
        process/io/InputSource.cpp process/io/InputSource.h
        common/IOHelper.cpp common/IOHelper.h
        common/StringHelper.cpp common/StringHelper.h
        process/io/S3Config.cpp process/io/S3Config.h
        process/io/S3InputSource.cpp process/io/S3InputSource.h
        process/io/StreamInputSource.cpp process/io/StreamInputSource.h
        process/io/PathInputSource.cpp process/io/PathInputSource.h
        runners/Runner.cpp runners/Runner.h
        startup/Starter.cpp startup/Starter.h
        startup/ModeCLIParser.cpp startup/ModeCLIParser.h
        startup/IndexModeCLIParser.cpp startup/IndexModeCLIParser.h
        startup/ExtractModeCLIParser.cpp startup/ExtractModeCLIParser.h
        process/base/ZLibBasedFASTQProcessorBaseClass.cpp process/base/ZLibBasedFASTQProcessorBaseClass.h
        runners/IndexStatsRunner.cpp runners/IndexStatsRunner.h)

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
