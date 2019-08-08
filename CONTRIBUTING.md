# Contributing to FastqIndEx

If you would like to contribute code you can do so through GitHub by forking the repository and sending us a pull request.

When submitting code, please make every effort to follow existing conventions and style in order to keep the code as readable as possible.

Please also try to write unit tests wherever it is possible.

## License

By contributing your code, you agree to license your contribution under the terms of the MIT License:

https://opensource.org/licenses/mit-license.html
https://github.com/DKFZ-ODCF/FastqIndEx/blob/master/LICENSE.txt

If you are adding a new file it should have a header like this:

```
/**
 * Copyright (c) 2019 DKFZ - ODCF
 *
 * Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqIndEx/blob/master/LICENSE.txt).
 */
 ```

## FASTQ Index (FQI) Format, Version 1

FQI files are binary files with the following general structure:

```
|HEADER|ENTRY 0|ENTRY 1|ENTRY 2|ENTRY ...|
```

The version 1 header is exactly 512 Byte wide and can be described like:

``` bash
|                                                                              (IndexHeader)                                                                             | 
|    (u_int32_t)     |    (u_int32_t)   | (u_int32_t) |  (u_int32_t)  |   (int64_t)   |     (int64_t)    |           (bool)          |   (u_char)  | (int64_t)[59] |
| indexWriterVersion | sizeOfIndexEntry | magicNumber | blockInterval | numberOfEntries | linesInIndexedFile | dictionariesAreCompressed | placeholder |     reserved    |
```

The version 1 index entry has an extracted width of 32800 Byte index entry can be described like:

```bash
|                                                                          IndexEntry                                                                         |
|   (int64_t)   |      (int64_t)     |     (int64_t)     |      (u_int32_t)       | (u_int32_t) | (u_char) |        (u_int16_t)       | (u_char)[32768] |
|     blockID     | blockOffsetInRawFile | startingLineInEntry | offsetOfFirstValidLine |     bits    | reserved | compressedDictionarySize |    dictionary   |
```

If compression is enabled, this looks a bit different (note the last field differs then and depends on compressedDictionarySize!), when it is stored in the FQI file:

```bash
|                                                                                   IndexEntry                                                                                   |
|   (int64_t)   |      (int64_t)     |     (int64_t)     |      (u_int32_t)       | (u_int32_t) | (u_char) |        (u_int16_t)       | (u_char)[compressedDictionarySize] |
|     blockID     | blockOffsetInRawFile | startingLineInEntry | offsetOfFirstValidLine |     bits    | reserved | compressedDictionarySize |             dictionary             |
```

If compression is enabled, you need to read out the IndexEntry without the dictionary first. 

## Development setup

**<span style="color:ffffa0;">If you use an IDE like CLion, make sure to 
activate the environment before running the IDE.</span>**

**<span style="color:ffffa0;">Also make sure to use the right compilers
and tools. They are named a bit differently in Conda. CLion recognizes
them, if you use the environment like mentioned.</span>**

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
- flock only works correctly with NFS on newer Linux kernels.
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

## Links

* [AWS](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup.html)
* [CMake](https://cmake.org/)
* [CMake Tutorial 1](https://preshing.com/20170511/how-to-build-a-cmake-based-project/)
* [CMake Tutorial 2](http://wiki.ogre3d.org/Getting+Started+With+CMake)
* [Miniconda](https://docs.conda.io/en/latest/miniconda.html)
* [UnitTest++](https://github.com/unittest-cpp/unittest-cpp)
* [Valgrind](http://valgrind.org/)
* [zindex](https://mattgodbolt.github.io/zindex/#/)
* [zran.c](https://github.com/madler/zlib/blob/master/examples/zran.c) 
  random gz file access example.