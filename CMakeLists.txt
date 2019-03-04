cmake_minimum_required(VERSION 3.13)
#
# Copyright (c) 2018 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#

# See https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests

#set(BOOST_ROOT "/data/michael/Projekte/boost_1_69_0") # either set it here or from the command line
# Need to figure out, how to do this in a portable way.
#set(BOOST_ROOT "/data/michael/Projekte/boost_1_64_0") # either set it here or from the command line
set(CMAKE_CXX_STANDARD 14)

project(FASTQIndEx)

find_package(Boost 1.54.0 COMPONENTS filesystem program_options system REQUIRED)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

