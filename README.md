# FastqIndEx
*"A tool to index and extract data from gzipped FASTQ files"*

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

FastqIndEx has the following dependencies:

| Dependency    | Version  / Git Hash                       | Remarks                                 |
| ---           |---                                        | ---                                     |
| CMake         | 3.13.4                                    |                                         |
| Boost         | 1.54                                      | Could not get higher version running.   |
| zlib          | 1.2.8                                     |                                         |
| UnitTest++    | bc5d87f484cac2959b0a0eafbde228e69e828d74  |                                         |

To compile it, create a CMake build directory and run CMake afterwards:

```
cd FastqIndEx
mkdir build
cd build
cmake -G "Unix Makefiles" -D BOOST_ROOT:PATH=/path/to/boost_1_69_0 -D "UnitTest++_DIR":PATH=/path/to/unittest-cpp ..
cd ..
cmake --build build --target all -- -j 2
```

To clean the build directory use:

```
cmake --build build --target clean -- -j 2
```

To run the tests, run the test binary like:

```
(cd build/test && ./testapp)
```

The tests take around 10sec on my machine.

## General Usage

### Index

```

```

### Extract

```
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
