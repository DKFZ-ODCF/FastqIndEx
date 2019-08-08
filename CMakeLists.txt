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

set(CMAKE_EXE_LINKER_FLAGS " -static-libstdc++ -static-libgcc")
#add_definitions(" -DCURL_STATICLIB")
#set(CURL_STATICLIB true)
#set(CMAKE_EXE_LINKER_FLAGS -static)

#-Wall
# See https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests
project(FASTQIndEx)

# General libraries

# Download and place next to FastqIndEx
set(SIMPLEINI_INCLUDE_DIR ../simpleini_4.17/)
set(AWS_DIR ../aws-sdk-cpp/install_shared)
set(AWSSDK_DIR "${AWS_DIR}/lib64/cmake/AWSSDK")
set(aws-cpp-sdk-core_DIR ${AWS_DIR}/lib64/cmake/aws-cpp-sdk-core)
set(aws-cpp-sdk-config_DIR ${AWS_DIR}/lib64/cmake/aws-cpp-sdk-config)
set(aws-c-event-stream_DIR ${AWS_DIR}/lib64/aws-c-event-stream/cmake)
set(aws-c-common_DIR ${AWS_DIR}/lib64/aws-c-common/cmake)
set(aws-checksums_DIR ${AWS_DIR}/lib64/aws-checksums/cmake)
set(aws-cpp-sdk-s3_DIR ${AWS_DIR}/lib64/cmake/aws-cpp-sdk-s3)

# Make cmake aware of an existing conda environment and try to load zlib and tclap from there.
# The variable CONDA_DEFAULT_ENV is set, if a Conda environment is active.
set(CONDA_DEFAULT_ENV $ENV{CONDA_DEFAULT_ENV})
set(CONDA_PREFIX $ENV{CONDA_PREFIX})
set(TCLAP_INCLUDE_DIR ../tclap-1.2.1/include)

# Conda overrides - Really, use conda if possible!
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

find_package(AWSSDK REQUIRED s3 )

## Amazon S3 Libraries
message(STATUS "AWSSDK_LIB_DIR ${AWSSDK_LIB_DIR}")
message(STATUS "AWSSDK_BIN_DIR ${AWSSDK_BIN_DIR}")
message(STATUS "AWSSDK_INCLUDE_DIR ${AWSSDK_INCLUDE_DIR}")
# https://github.com/aws/aws-sdk-cpp/issues/838
SET(SERVICE "s3")
AWSSDK_DETERMINE_LIBS_TO_LINK(SERVICE OUTPUT)
message(STATUS "All linked libs: ${OUTPUT}")
message("All AWS libs: ${AWSSDK_LINK_LIBRARIES}")

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
