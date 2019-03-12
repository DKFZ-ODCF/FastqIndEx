# FastqIndEx

## Description

FastqIndEx allows you to create an index file for gzip compressed FASTQ
to enable random access to the FASTQ file. Though its primary goal is to
extract data from FASTQ files, you can also use it to index gzipped text
files.

## License 

FastqIndEx is published under the MIT license. See 
[LICENSE.txt](LICENSE.txt) for more information.

## Installation

### Specific releases

We will provide you with stable releases. Please go to the 
[releases section](https://github.com/DKFZ-ODCF/FastqIndEx/releases).

Download the desired package and extract it to a location of your 
choice and run it.

### Download + Compile


#### Downloads and install dependencies, if necessary (before trying to make the binary)

FastqIndEx has the following dependencies, which should be met before building it:

| Dependency    | Version  / Git Hash                       | Remarks                                 |
| ---           |---                                        | ---                                     |
| Boost         | 1.54                                      | Could not get higher version running.   |
| CMake         | 3.13.x                                    |                                         |
| gcc           | 4.8 / 7.3                                 | Build was tested with both versions.    |
| zlib          | 1.2.8 / 1.2.11                            | Build was tested with both versions.    |
| UnitTest++    | bc5d87f484cac2959b0a0eafbde228e69e828d74  |                                         |

##### Boost, CMake, zlib...

``` Bash
wget https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4.tar.gz
tar -xvzf cmake-3.13.4.tar.gz
# Configure and build afterwards

wget https://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.gz/download && mv download boost_1_54_0.tar.gz
tar -xvzf boost_1_54_0.tar.gz
# Configure and build afterwards

wget https://www.zlib.net/zlib-1.2.11.tar.gz
tar -xvzf zlib-1.2.11.tar.gz
cd zlib-1.2.11 && ./configure && make
```

##### UnitTest++

``` Bash
git clone https://github.com/unittest-cpp/unittest-cpp.git
cd unittest-cpp
mkdir builds
cd builds
cmake -G "Unix Makefiles" -D CMAKE_INSTALL_PREFIX=/custom/lib/path ..
cmake --build . --target all
cmake --build . --target install
```

#### FastqIndEx


To compile it, create a CMake build directory and run CMake afterwards:

``` Bash
cd FastqIndEx
mkdir release                                                             # Or also debug, in case you want to develop
cd release
cmake -G "Unix Makefiles" \
    -D BOOST_ROOT:PATH=/path/to/boost_1_54_0 \                            # If necessary
    -D "UnitTest++_DIR":PATH=/path/to/unittest-cpp/lib/cmake/UnitTest++ \ # If necessary
    -D ZLIB_LIBRARARY=/path/to/zlib-1.2.11/libz.a \                       # If necessary
    -D ZLIB_INCLUDE_DIR=/path/to/zlib-1.2.11 \                            # If necessary
    -DCMAKE_BUILD_TYPE=Release                                            # Or =Debug, if you plan to develop 
    ..
cd ..
cmake --build release --target all -- -j 2                                # Or --build debug
```

To clean the build directory use:

``` Bash
cmake --build build --target clean -- -j 2
```

To run the tests, run the test binary like:

``` Bash
(cd build/test && ./testapp)
```

The tests take around 10sec on my machine.



## General Usage

### Index

``` Bash

```

### Extract

``` Bash

```

## Further links

* [Boost](https://www.boost.org/)
* [CMake](https://cmake.org/)
* [CMake Tutorial 1](https://preshing.com/20170511/how-to-build-a-cmake-based-project/)
* [CMake Tutorial 2](http://wiki.ogre3d.org/Getting+Started+With+CMake)
* [UnitTest++](https://github.com/unittest-cpp/unittest-cpp)
* [zindex](https://mattgodbolt.github.io/zindex/#/)
* [zran.c](https://github.com/madler/zlib/blob/master/examples/zran.c) 
  random gz file access example.

