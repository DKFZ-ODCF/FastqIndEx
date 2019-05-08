#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_EXE_LINKER_FLAGS -static)

add_executable(fastqindex main.cpp main.h)

add_library(
        fastqindexlib
        ActualRunner.cpp ActualRunner.h
        CommonStructsAndConstants.cpp CommonStructsAndConstants.h
        ErrorAccumulator.cpp ErrorAccumulator.h
        ErrorMessages.cpp ErrorMessages.h
        Extractor.cpp Extractor.h
        ExtractorRunner.cpp ExtractorRunner.h
        IndexerRunner.cpp IndexerRunner.h
        Indexer.cpp Indexer.h
        IndexReader.cpp IndexReader.h
        IndexWriter.cpp IndexWriter.h
        IndexProcessor.cpp IndexProcessor.h
        InputSource.cpp InputSource.h
        StreamInputSource.cpp StreamInputSource.h
        PathInputSource.cpp PathInputSource.h
        Runner.cpp Runner.h
        Starter.cpp Starter.h
        ZLibBasedFASTQProcessorBaseClass.cpp ZLibBasedFASTQProcessorBaseClass.h
        IndexStatsRunner.cpp IndexStatsRunner.h)

target_link_libraries(
        fastqindexlib
        LINK_PUBLIC
        stdc++fs
        ${CMAKE_THREAD_LIBS_INIT}
        ${ZLIB_LIBRARY}
)

target_link_libraries(
        fastqindex
        fastqindexlib
)
