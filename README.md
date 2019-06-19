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

#### Resolve dependencies

FastqIndEx has the following dependencies, which should be met before building it:

| Dependency    | Version  / Git Hash | Conda | Remarks |
| ---           |---                  | ---   | ---     |
| CMake         | 3.13.x              | yes   |         |
| gcc           | 7.2                 | yes   |         |
| tclap         | 1.2.1               | yes   |         |
| zlib          | 1.2.11              | yes   |         |
| UnitTest++    | bc5d87f             | no    |         |

##### Use Conda to manage your dependencies

* Install Miniconda / Anaconda
* The conda recipe is contained in env/environmenty.yml, use conda-env 
  to import it.
* Download and install UnitTest++ like described in the next section, 
  it is not available in Conda.

**<span style="color:ffffa0;">If you use an IDE like CLion, make sure to 
activate the environment before running the IDE.</span>**

**<span style="color:ffffa0;">Also make sure to use the right compilers
and tools. They are named a bit differently in Conda. CLion recognizes
them, if you use the environment like mentioned.</span>**

##### Compilation with manual download and installation of dependencies

###### g++/gcc

Before you run cmake, you might need to set

``` Bash
export CC=/usr/local/bin/gcc
export CXX=/usr/local/bin/g++
```

to the proper locations of your gcc/g++ binaries. 

###### CMake, zlib...

``` Bash
wget https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4.tar.gz
tar -xvzf cmake-3.13.4.tar.gz
# Configure and build afterwards

wget https://www.zlib.net/zlib-1.2.11.tar.gz
tar -xvzf zlib-1.2.11.tar.gz
cd zlib-1.2.11 && ./configure && make
```

###### UnitTest++

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

First, you need to clone the FastqIndEx Git repository. You can do this
by executing:

``` bash
cd ~/Projects           # Or any other desired location, we'll stick to this
git clone https://github.com/DKFZ-ODCF/FastqIndEx.git
git checkout master     # Or any other version you like
```

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

If you want, you can add the release or debug directory to your PATH
variable. E.g. in your local .bashrc file add the following:

``` bash
# This assumes, that you cloned the repo to ~/Projects/FastqIndEx and 
# created the release sub directory like described above.
export PATH=~/Projects/FastqIndEx/release/src:$PATH
```

## General Usage

### Index

``` Bash
# Index from a file
fastqindex index -f=test2.fastq.gz -i=test2.fastq.gz.fqi

# Index from a pipe
cat test2.fastq.gz | fastqindex -f=- -i=test2.fastq.fqi
```

There are more options available like:

| Option        | Description         |
| ---           |---                  |
| -b            | Defines, that an index entry will be written to the index file for every nth compressed block. Value needs to be in range from [1 .. n]. By default, the block distance will be determined by the application. |
| -w            | Allow the application to overwrite the index file. By default, this is not allowed. |

Please call the application with 
``` bash
fastqindex index
```
to see more options.

### Extract

``` Bash
# Extract to a file, note, that this command produces an uncompressed file!
fastqindex extract -f=test2.fastq.gz -i=test2.fastq.fqi -o=extracted.fastq

# Or to stdout / the console, this is also uncompressed data!
fastqindex extract -f=test2.fastq.gz -i=test2.fastq.fqi -o=- 
```

There are more options available here as well like:

| Option        | Description         |
| ---           |---                  |
| -s            | Defines the first line (multiplied by extractionmultiplier) which. By default this aligns to each read, assuming a read has a size of four lines. |
| -n            | Defines the number of reads which should be extracted. The size of each read is defined by extractionmultiplier. |
| -e            | Defines a multiplier by which the startingline parameter will be mulitplied. For FASTQ files this is 4 (record size), but you could use 1 for e.g. regular text files. |
| -w            | Allow the application to overwrite the index file. By default, this is not allowed. |

Please call the application with 
``` bash
fastqindex extract
```
to see more options.

## FQI format description

FQI files are binary files which look like:

```
|HEADER|ENTRY 0|ENTRY 1|ENTRY 2|ENTRY ...|
```

The V1 header is exactly 512 Byte wide and can be described like:

``` bash
|                                                                              (IndexHeader)                                                                             | 
|    (u_int32_t)     |    (u_int32_t)   | (u_int32_t) |  (u_int32_t)  |   (u_int64_t)   |     (u_int64_t)    |           (bool)          |   (u_char)  | (u_int64_t)[59] |
| indexWriterVersion | sizeOfIndexEntry | magicNumber | blockInterval | numberOfEntries | linesInIndexedFile | dictionariesAreCompressed | placeholder |     reserved    |
```

The V1 index entry has an extracted width of 32800 Byte index entry can be described like:

```bash
|                                                                          IndexEntry                                                                         |
|   (u_int64_t)   |      (u_int64_t)     |     (u_int64_t)     |      (u_int32_t)       | (u_int32_t) | (u_char) |        (u_int16_t)       | (u_char)[32768] |
|     blockID     | blockOffsetInRawFile | startingLineInEntry | offsetOfFirstValidLine |     bits    | reserved | compressedDictionarySize |    dictionary   |
```

If compression is enabled, this looks a bit different (note the last field differs then and depends on compressedDictionarySize!), when it is stored in the fqi file:

```bash
|                                                                                   IndexEntry                                                                                   |
|   (u_int64_t)   |      (u_int64_t)     |     (u_int64_t)     |      (u_int32_t)       | (u_int32_t) | (u_char) |        (u_int16_t)       | (u_char)[compressedDictionarySize] |
|     blockID     | blockOffsetInRawFile | startingLineInEntry | offsetOfFirstValidLine |     bits    | reserved | compressedDictionarySize |             dictionary             |
```

If compression is enabled, you need to read out the IndexEntry without the dictionary first. 

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

A tool to index and extract data from gzipped FASTQ files.

Current development is happening in InitialCommit, the master branch will be "empty" until we release a first working version.
