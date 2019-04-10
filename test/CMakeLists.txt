#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXE_LINKER_FLAGS -static)

# See: https://github.com/unittest-cpp/unittest-cpp/wiki
find_package(UnitTest++ REQUIRED)

include_directories(
        ${TEST_SOURCE_DIR}/src
        ${CTEST_SOURCE_DIR}/src
        ${UTPP_INCLUDE_DIRS}
)

add_executable(
        testapp
        main.cpp
        ActualRunnerTest.cpp
        CommonStructsTest.cpp
        ExtractorRunnerTest.cpp
        ExtractorTest.cpp
        IndexerRunnerTest.cpp
        IndexerTest.cpp
        IndexProcessorTest.cpp
        IndexReaderTest.cpp
        IndexWriterTest.cpp
        IntegrationTests.cpp
        ShowStopperRunnerTest.cpp
        StarterTest.cpp
        TestResourcesAndFunctions.cpp TestResourcesAndFunctions.h
        TestResourcesAndFunctionsTest.cpp
        TestConstants.cpp TestConstants.h ZLibBasedFASTQProcessorBaseClassTest.cpp)

target_link_libraries(
        testapp
        LINK_PUBLIC
        fastqindexlib
        UnitTest++
)

add_test(NAME AllTests COMMAND testapp)
