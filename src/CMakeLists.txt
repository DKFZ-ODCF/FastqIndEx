#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#

find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
find_package(Threads)
include_directories(
        ${TEST_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
)

add_executable(fastqindex main.cpp main.h)

add_library(
        fastqindexlib
        CommonStructsAndConstants.h
        Starter.cpp Starter.h
        Runner.cpp Runner.h
        ActualRunner.cpp ActualRunner.h
        IndexerRunner.cpp IndexerRunner.h
        ExtractorRunner.cpp ExtractorRunner.h
        ErrorMessages.cpp
        ErrorMessages.h
        Indexer.cpp Indexer.h
        IndexReader.cpp IndexReader.h
        IndexWriter.cpp IndexWriter.h IndexProcessor.cpp IndexProcessor.h CommonStructsAndConstants.cpp ErrorAccumulator.cpp ErrorAccumulator.h)

target_link_libraries(
        fastqindexlib
        LINK_PUBLIC
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(
        fastqindex
        fastqindexlib
)