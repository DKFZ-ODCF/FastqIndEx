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

include_directories(
        ${CMAKE_SOURCE_DIR}/src
)

add_library(
        fastqindexlib
        common/CommonStructsAndConstants.cpp common/CommonStructsAndConstants.h
        common/ErrorAccumulator.cpp common/ErrorAccumulator.h
        common/ErrorMessages.cpp common/ErrorMessages.h
        common/IOHelper.cpp common/IOHelper.h
        common/StringHelper.cpp common/StringHelper.h
        process/base/ZLibBasedFASTQProcessorBaseClass.cpp process/base/ZLibBasedFASTQProcessorBaseClass.h
        process/extract/Extractor.cpp process/extract/Extractor.h
        process/extract/IndexReader.cpp process/extract/IndexReader.h
        process/index/Indexer.cpp process/index/Indexer.h
        process/index/IndexWriter.cpp process/index/IndexWriter.h
        process/io/s3/FQIS3Client.h
        process/io/s3/S3ServiceOptions.h
        process/io/s3/S3Config.cpp process/io/s3/S3Config.h
        process/io/s3/S3Service.cpp process/io/s3/S3Service.h
        process/io/s3/S3Sink.h
        process/io/s3/S3Source.cpp process/io/s3/S3Source.h
        process/io/locks/LockHandler.h
        process/io/locks/PathLockHandler.cpp process/io/locks/PathLockHandler.h
        process/io/locks/S3LockHandler.h
        process/io/IOBase.h
        process/io/PathSink.cpp process/io/PathSink.h
        process/io/PathSource.cpp process/io/PathSource.h
        process/io/Sink.h
        process/io/Source.h
        process/io/ConsoleSink.h
        process/io/StreamSource.cpp process/io/StreamSource.h
        runners/ActualRunner.cpp runners/ActualRunner.h
        runners/ExtractorRunner.cpp runners/ExtractorRunner.h
        runners/IndexerRunner.cpp runners/IndexerRunner.h
        runners/IndexStatsRunner.cpp runners/IndexStatsRunner.h
        runners/Runner.cpp runners/Runner.h
        runners/DoNothingRunner.cpp runners/DoNothingRunner.h
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
