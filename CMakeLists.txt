#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
# -pedantic -ansi -Winit-self -Wmissing-declarations -Wextra -Winit-self -Wold-style-cast -Woverloaded-virtual -Wuninitialized
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fPIC")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fPIC")
SET(CMAKE_EXE_LINKER_FLAGS " -static-libstdc++ -static-libgcc")

# See https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests
project(FASTQIndEx)

# General libraries

set(SIMPLEINI_INCLUDE_DIR $ENV{SIMPLEINI_INCLUDE_DIR})
set(TCLAP_INCLUDE_DIR $ENV{TCLAP_INCLUDE_DIR})

set(ZLIB_LIBRARY      "${CONDA_PREFIX}/lib/libz.a")
set(ZLIB_INCLUDE_DIR  "${CONDA_PREFIX}/include")
set(TCLAP_INCLUDE_DIR "${CONDA_PREFIX}/include/tclap")

message("  TCLAP:    ${TCLAP_INCLUDE_DIR}")
message("  ZLIB")
message("   library: ${ZLIB_LIBRARY}")
message("   include: ${ZLIB_INCLUDE_DIR}")

find_package(AWSSDK REQUIRED s3 )

## Amazon S3 Libraries
SET(SERVICE "s3")
AWSSDK_DETERMINE_LIBS_TO_LINK(SERVICE OUTPUT)
message(STATUS "All linked libs: ${OUTPUT}")
message("All AWS libs: ${AWSSDK_LINK_LIBRARIES}")
message("AWS library dir: ${AWSSDK_LIB_DIR}")
link_directories("${AWSSDK_LIB_DIR}")
include_directories(BEFORE ${AWSSDK_INCLUDE_DIR})

# See https://stackoverflow.com/questions/25213659/how-to-use-tclap-in-cmake-project
# As you can see, we use tclap-1.2.1, download this version and extract to the project root directory, if you do not
# use Conda.
include_directories(BEFORE ${TCLAP_INCLUDE_DIR})
include_directories(BEFORE ${ZLIB_INCLUDE_DIR})
include_directories(BEFORE ${SIMPLEINI_INCLUDE_DIR})

find_package(ZLIB 1.2.11 REQUIRED)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
