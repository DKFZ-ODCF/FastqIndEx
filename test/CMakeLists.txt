#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#

find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
# See: https://github.com/unittest-cpp/unittest-cpp/wiki
# Could not get Boost tests to run properly. Either only the first added test file was executed or there were
# build errors like multiple main methods or no methods or no test tree. The list is long.
find_package(UnitTest++ REQUIRED)

include_directories(
        ${TEST_SOURCE_DIR}/src
        ${CTEST_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
        ${UTPP_INCLUDE_DIRS}
)

add_executable(
        testapp
        main.cpp
        ActualRunnerTest.cpp
        CommonStructsTest.cpp
        ExtractorRunnerTest.cpp
        IndexerRunnerTest.cpp
        IndexerTest.cpp
        IndexProcessorTest.cpp
        IndexReaderTest.cpp
        IndexWriterTest.cpp
        ShowStopperRunnerTest.cpp
        StarterTest.cpp
        TestResourcesAndFunctions.cpp TestResourcesAndFunctions.h
        TestResourcesAndFunctionsTest.cpp
        TestConstants.cpp TestConstants.h)

target_link_libraries(
        testapp
        LINK_PUBLIC
        fastqindexlib
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LIBRARIES}
        UnitTest++
)

add_test(NAME AllTests COMMAND testapp)