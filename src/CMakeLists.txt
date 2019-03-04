cmake_minimum_required(VERSION 3.13)
#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
set(CMAKE_CXX_STANDARD 14)
find_package(Boost 1.54.0 COMPONENTS filesystem program_options system REQUIRED)
find_package(Threads)
find_package(ZLIB REQUIRED)
#if (ZLIB_FOUND)
#    include_directories(${ZLIB_INCLUDE_DIRS})
#endif ()

include_directories(
        ${TEST_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
)

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
        ErrorAccumulator.cpp ErrorAccumulator.h Extractor.cpp Extractor.h BaseExtractor.h)

target_link_libraries(
        fastqindexlib
        LINK_PUBLIC
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${ZLIB_LIBRARIES}
)

target_link_libraries(
        fastqindex
        fastqindexlib
)
