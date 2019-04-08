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

FastqIndEx has the following dependencies, which should be met before building it:

| Dependency    | Version  / Git Hash | Conda | Remarks |
| ---           |---                  | ---   | ---     |
| CMake         | 3.13.x              | yes   |         |
| gcc           | 7.2                 | yes   |         |
| tclap         | 1.2.1               | yes   |         |
| zlib          | 1.2.11              | yes   |         |
| UnitTest++    | bc5d87f             | no    |         |

#### Compilation with manual download and installation of dependencies

##### g++/gcc

Before you run cmake, you might need to set

``` Bash
export CC=/usr/local/bin/gcc
export CXX=/usr/local/bin/g++
```

to the proper locations of your gcc/g++ binaries. 

##### CMake, zlib...

``` Bash
wget https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4.tar.gz
tar -xvzf cmake-3.13.4.tar.gz
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

#### Use Conda to manage your dependencies

* Install Miniconda / Anaconda
* The conda recipe is contained in env/environmenty.yml, use conda-env 
  to import it.
* Download and install UnitTest++ like above, it is not available in 
  Conda.

**<span style="color:ffffa0;">If you use an IDE like CLion, make sure to 
activate the environment before running the IDE.</span>**

**<span style="color:ffffa0;">Also make sure to use the right compilers
and tools. They are named a bit differently in Conda. CLion recognizes
them, if you use the environment like mentioned.</span>**

#### FastqIndEx

To compile it, create a CMake build directory and run CMake afterwards:

``` Bash
cd FastqIndEx
mkdir release                                                             # Or also debug, in case you want to develop
cd release
cmake -G "Unix Makefiles" \
    -D "UnitTest++_DIR":PATH=/path/to/unittest-cpp/lib/cmake/UnitTest++ \ # If necessary
    -D ZLIB_LIBRARY=/path/to/zlib-1.2.11/libz.a \                         # If necessary
    -D ZLIB_INCLUDE_DIR=/path/to/zlib-1.2.11 \                            # If necessary
    -DCMAKE_BUILD_TYPE=Release                                            # Or =Debug, if you plan to develop 
    ..
cd ..
cmake --build release --target all -- -j 2                                # Or --build debug
```

**<span style="color:ffffa0;">
Note, that the -D flags for the includes are only necessary, if you
manually installed the libraries. If they are already installed on your 
system or (e.g. for UnitTest++) you installed them to the system folders
or if you use the Conda environment, you can omit these flags.
</span>**

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
/home/heinold/Projekte/FastqIndEx/release/src/fastqindex index -f=test4.fastq.gz -i=test4.fastq.gz.idx2
```

### Extract

``` Bash

```

## Code stability and safety

There are several things we consider and do to make usage of FastqIndEx
as safe as possible.

   
**<span style="color:orange;">=>  We will not get the application a 100%
  safe, but we try to minimize the risk of data corruption.</span>**

### Code validation with clang-tidy

We use CLion to develop FastqIndEx. CLion has clang-tidy support built 
in, which we use to eliminate well known issues and problems.

### Runtime checks with Valgrind

We use [Valgrind](http://valgrind.org/) to check for memory leaks which 
e.g. lead to SIGSEV or SIGABRT.

### Index file access considerations

When it comes to safely accessing the index file, we want to have it so
that:
- Writing to the index is exclusive. No other reader or writer is 
  allowed at the same time.
- It is allowed to read multiple times from an index file, if no writer 
  is active. 

As we work with network file systems, we need to deal with several 
problems:
- flock only works correct with NFS on newer Linux kernels, which is ok.
- However, we cannot guarantee, that a file which was not locked, will
  be overriden during our read.

To overcome these problems:
- We can use flock for our writer. So writing will be safe, as long as
  no other process starts writing to the file. 
  <br><span style="color:green;">=> Implemented</span>
- We can write out a md5sum file for the index after it was created. 
  <br><span style="color:red;">Not implemented</span>
- We can check for an existing lockfile before we read from a file and 
  abort early. 
  <br><span style="color:green;">Implemented</span>
- We can read in the md5sum file and calculate our md5sum during our 
  read. 
  <br><span style="color:red;">Not implemented</span> 
- We can also constantly perform sanity check for changes in timestamp,
  file size or file existence. 
  <br><span style="color:red;">Not implemented</span>
- We can store md5sums for IndexEntries, e.g. for the first valid 
  line(s) in each compressed block. 
  <br><span style="color:red;">Not implemented</span>

## Further links

* [CMake](https://cmake.org/)
* [CMake Tutorial 1](https://preshing.com/20170511/how-to-build-a-cmake-based-project/)
* [CMake Tutorial 2](http://wiki.ogre3d.org/Getting+Started+With+CMake)
* [UnitTest++](https://github.com/unittest-cpp/unittest-cpp)
* [Valgrind](http://valgrind.org/)
* [zindex](https://mattgodbolt.github.io/zindex/#/)
* [zran.c](https://github.com/madler/zlib/blob/master/examples/zran.c) 
  random gz file access example.

