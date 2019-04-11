#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fPIC")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fPIC")

# See https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests
project(FASTQIndEx)

# Make cmake aware of an existing conda environment and try to load zlib and tclap from there.
set(CONDA_DEFAULT_ENV $ENV{CONDA_DEFAULT_ENV})
set(CONDA_PREFIX $ENV{CONDA_PREFIX})
set(TCLAP_INCLUDE_DIR ../tclap-1.2.1/include)

if (CONDA_DEFAULT_ENV)
    set(ZLIB_LIBRARY "${CONDA_PREFIX}/lib/libz.a")
    set(ZLIB_INCLUDE_DIR "${CONDA_PREFIX}/include")
    set(TCLAP_INCLUDE_DIR "${CONDA_PREFIX}/include/tclap")
    message("Loading a conda environment '${CONDA_DEFAULT_ENV}':")
    message("  Prefix:   ${CONDA_PREFIX}")
    message("  TCLAP:    ${TCLAP_INCLUDE_DIR}")
    message("  ZLIB")
    message("   library: ${ZLIB_LIBRARY}")
    message("   include: ${ZLIB_INCLUDE_DIR}")
endif (CONDA_DEFAULT_ENV)

include_directories(BEFORE ${ZLIB_INCLUDE_DIR})

# See https://stackoverflow.com/questions/25213659/how-to-use-tclap-in-cmake-project
# As you can see, we use tclap-1.2.1, download this version and extract to the project root directory, if you do not
# use Conda.
include_directories(BEFORE ${TCLAP_INCLUDE_DIR})

find_package(ZLIB 1.2.11 REQUIRED)


add_subdirectory(src)

enable_testing()
add_subdirectory(test)

