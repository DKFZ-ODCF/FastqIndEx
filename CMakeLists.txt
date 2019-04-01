#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

# See https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests
project(FASTQIndEx)

include_directories( BEFORE tclap-1.2.1/include )

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

