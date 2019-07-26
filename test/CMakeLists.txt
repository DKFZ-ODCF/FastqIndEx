#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

#add_definitions(" -DCURL_STATICLIB")
#set(CURL_STATICLIB true)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -lcrypto -lssl -lz -lcurl")

#set(CMAKE_EXE_LINKER_FLAGS -static)

set(UnitTest++_DIR ../../unittest-cpp/install/lib/cmake/UnitTest++)
# See: https://github.com/unittest-cpp/unittest-cpp/wiki

find_package(UnitTest++ REQUIRED)

include_directories(
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/test
        ${UTPP_INCLUDE_DIRS}
)

MESSAGE("${CMAKE_SOURCE_DIR}/test")

add_executable(
        testapp
        main.cpp
        TestConstants.cpp TestConstants.h
        TestResourcesAndFunctions.cpp TestResourcesAndFunctions.h
        TestResourcesAndFunctionsTest.cpp

        # Leave these two right at the beginning, these tests are crucial
        process/io/PathSinkTest.cpp
        process/io/PathSourceTest.cpp

        process/index/IndexWriterTest.cpp
        process/index/IndexerTest.cpp
        process/index/IndexEntryStorageStrategyTest.cpp

        process/io/SourceTest.cpp
        common/CommonStructsTest.cpp
        common/IOHelperTest.cpp
        common/StringHelperTest.cpp
        process/base/ZLibBasedFASTQProcessorBaseClassTest.cpp
        process/extract/ExtractorTest.cpp
        process/extract/IndexReaderTest.cpp
        process/io/locks/LockHandlerTest.cpp
        process/io/ConsoleSinkTest.cpp
        process/io/s3/S3ConfigTest.cpp
        process/io/s3/S3SinkTest.cpp
        process/io/StreamSourceTest.cpp
        runners/ActualRunnerTest.cpp
        runners/ExtractorRunnerTest.cpp
        runners/IndexerRunnerTest.cpp
        runners/IndexStatsRunnerTest.cpp
        runners/ShowStopperRunnerTest.cpp
        startup/ModeCLIParserTest.cpp
        startup/StarterTest.cpp
)

target_link_libraries(
        testapp
        LINK_PUBLIC
        fastqindexlib
        UnitTest++
)

add_test(NAME AllTests COMMAND testapp)
