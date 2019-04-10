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
        CommonStructsAndConstants.cpp CommonStructsAndConstants.h
        Starter.cpp Starter.h
        Runner.cpp Runner.h
        ActualRunner.cpp ActualRunner.h
        IndexerRunner.cpp IndexerRunner.h
        ExtractorRunner.cpp ExtractorRunner.h
        ErrorMessages.cpp ErrorMessages.h
        Indexer.cpp Indexer.h
        IndexReader.cpp IndexReader.h
        IndexWriter.cpp IndexWriter.h
        IndexProcessor.cpp IndexProcessor.h
        ErrorAccumulator.cpp ErrorAccumulator.h Extractor.cpp Extractor.h ZLibBasedFASTQProcessorBaseClass.cpp ZLibBasedFASTQProcessorBaseClass.h)

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
