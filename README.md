# FastqIndEx

## Description

FastqIndEx allows you to create an index file for gzip compressed FASTQ
to enable random access to the FASTQ file. Though its primary goal is to
extract data from FASTQ files, you can also use it to index gzipped text
files.

## Features at a glance

* Support for:
  - Local file access (either by piped or streamed access)
  - Safe concurrent file access over NFS (using flock(), also with 
    either piped or streamed access)
  - Files in an S3 bucket (experimental, without locking yet!)
* Many checks to make sure, that the application does what you expect
  and runs safely.
* A number of options to tell the indexer how you like to get things 
  done.
* A flexible and configurable index size.
* Different extraction strategies:
  - Extract a range of records 
  - (Virtually) divide the FASTQ on the fly into n segments and extract 
    one segement of your choice.

## License and Contributing 

FastqIndEx is published under the MIT license. See 
[LICENSE.txt](LICENSE.txt) for more information.

Please note, that we will only accept contributions, which are compatible
with the MIT license. We will therefore not accept contributions which
are e.g. licensed under GPL!

For more information on code contributions, please refer to 
[CONTRIBUTING.md](CONTRIBUTING.md)

## General Usage

Too see, how FastqIndEx is installed, please read the next section of
 this documentation.

### Index

``` Bash
# Index a file, default options, automatic distance calculation for index entries.
fastqindex index -f=test2.fastq.gz 

# Index piped input with a distance of 2GiB for index entries.
cat test2.fastq.gz | fastqindex -f=- -i=test2.fastq.fqi -B=2G

# Index an object stored in an S3 bucket, the index is stored in the Bucket!
fastqindex index -f=s3://bucket/test2.fastq.gz
```

There are more options available like:

| Option        | Description         |
| ---           |---                  |
| -w            | Allow the application to overwrite the index file. By default, this is not allowed. |
| -B            | Tell the indexer to store an entry after approximately n Byte (like 4M, 2G, 512K)|

Please call the application with 
``` bash
fastqindex index
```
to see more options.

### Extract

``` Bash
# Extract to a file, note, that this command produces an uncompressed file!
# Note, that the command will decompress everything (basically gunzip)
fastqindex extract -f=test2.fastq.gz -i=test2.fastq.fqi -o=extracted.fastq

# Or to stdout / the console, this is also uncompressed data!
# Extract 16 records, starting with record 201 (we have a 0 based offset!)
fastqindex extract -f=test2.fastq.gz -i=test2.fastq.fqi -o=- -s=200 -n=16

# Or also from S3 with a locally stored index.
# Extract the second and third record.
fastqindex extract -f=s3://bucket/test2.fastq.gz -i=/local/path/test2.fastq.fqi -o=- -s=1 -n=2
```
Please note, that the S3 extraction is still experimental (but working for us). Unfortunately, there
will always be an error message, that a stream was closed. You can ignore this.

There are more options available here as well like:

| Option        | Description         |
| ---           |---                  |
| -s            | Defines the first record which will be extracted. By default, the application assumes, that a record has 4 lines. |
| -n            | Defines the number of reads which should be extracted.  |
| -e            | Defines the size of a record. For FASTQ files this is 4 (record size), but you could use 1 for e.g. regular text files. |
| -w            | Allow the application to overwrite the index file. By default, this is not allowed. |

Please call the application with 
``` bash
fastqindex extract
```
to see more options.

## Installation

### Binary releases

We provide you with stable releases. Please go to the 
[releases section](https://github.com/DKFZ-ODCF/FastqIndEx/releases).

Download the desired package and extract it to a location of your 
choice and run it.

### Compilation

#### Dependencies

FastqIndEx has the following dependencies, which should be met before building it:

| Dependency    | Conda | Version  / Git Hash | Purpose                          |
| ---           | ---   | ---                 |  ---                             |
| CMake         | yes   | 3.13.x              | Build tool                       |
| gcc           | yes   | 7.2                 | Compiler and debugger suite      |
| tclap         | yes   | 1.2.1               | Command line interpreter library |
| zlib          | yes   | 1.2.11              | Compression library              |
| AWS SDK       | yes   | 1.7.160             | S3 Support library               |
| UnitTest++    | no    | bc5d87f             | Unit testing framework           |
| simpleini     | no    | 4.17                | Simple parser for header files   |

##### Use Conda to manage your dependencies

1. Install Miniconda / Anaconda
2. The conda recipe is contained in env/environment.yml, use conda-env 
   to import it:
   conda env create -n FastqIndEx -f env/environment.yml
3. Download and install UnitTest++ like described in the next section, 
   it is not available in Conda.

##### Compilation with manual installation of dependencies

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
rm cmake-3.13.4.tar.gz
# Configure and build afterwards

wget https://www.zlib.net/zlib-1.2.11.tar.gz
tar -xvzf zlib-1.2.11.tar.gz
(cd zlib-1.2.11 && ./configure && make)
rm zlib-1.2.11.tar.gz
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

###### simpleini / tclap

Place the directory alongside the FastqIndEx directory.

``` Bash
git clone -b "4.17" --single-branch --depth 1 https://github.com/brofield/simpleini.git simpleini_4.17
wget https://vorboss.dl.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz && tar -xvzf tclap-1.2.1.tar.gz && rm tclap-1.2.1.tar.gz 
```

##### AWS S3

Download the AWS SDK from the Amazon website and install it to a 
location of your choice. Compile as necessary and remember to first
activate the Conda environment of FastqIndEx.

#### FastqIndEx

1. First, you need to clone the FastqIndEx Git repository. You can do this
   by executing:

    ``` bash
    cd ~/Projects           # Or any other desired location, we'll stick to this
    git clone https://github.com/DKFZ-ODCF/FastqIndEx.git
    git checkout master     # Or any other version you like
    ```

2. To compile it, create a CMake build directory and run CMake afterwards:

    ``` Bash
    export CONDA_DIR="~/miniconda2/envs/FastqIndEx"     # Miniconda is normally installed in <home>/miniconda2
    export CONDA_LIB=${CONDA_DIR}/lib
    export CONDA_LIB64=${CONDA_DIR}/lib64
    export CONDA_INCLUDE=${CONDA_DIR}/include
    cmake -D CMAKE_BUILD_TYPE=Debug -D BUILD_ONLY="s3;config;transfer" -D BUILD_SHARED_LIBS=ON  -D OPENSSL_ROOT_DIR=${CONDA_DIR}  -D OPENSSL_INCLUDE_DIR=${CONDA_INCLUDE} OPENSSL_LIBRARIES=${CONDA_LIB}  -D ZLIB_INCLUDE_DIR=${CONDA_INCLUDE} -DZLIB_LIBRARY=${CONDA_LIB}/libz.a -D CURL_INCLUDE_DIR=${CONDA_INCLUDE} -DCURL_LIBRARY=${CONDA_LIB}/libcurl.so  -D CMAKE_INSTALL_PREFIX=$PWD/../install_shared -D ENABLE_TESTING=OFF ..
    cmake -D CMAKE_BUILD_TYPE=Debug -D BUILD_ONLY="s3;config;transfer" -D BUILD_SHARED_LIBS=OFF  -D OPENSSL_ROOT_DIR=${CONDA_DIR}  -D OPENSSL_INCLUDE_DIR=${CONDA_INCLUDE} OPENSSL_LIBRARIES=${CONDA_LIB}  -D ZLIB_INCLUDE_DIR=${CONDA_INCLUDE} -DZLIB_LIBRARY=${CONDA_LIB}/libz.a -D CURL_INCLUDE_DIR=${CONDA_INCLUDE} -DCURL_LIBRARY=${CONDA_LIB}/libcurl.so  -D CMAKE_INSTALL_PREFIX=$PWD/../install_shared -D ENABLE_TESTING=OFF ..
    
    
    export AWS_DIR="/path/to/aws-sdk-cpp/install_shared"
    export UNITTESTPP_DIR="/path/to/unittest-cpp"
    export ZLIB_DIR="/path/to/zlib-1.2.11"                                    # If necessary

    # The following instructions are for a release build of FastqIndEx. 
    # If you want to create a debug build, change "release" to "debug"
    export MODE=release
    cd FastqIndEx
    mkdir ${MODE}                                                             
    cd ${MODE}
    cmake -G "Unix Makefiles" \
        -D "UnitTest++_DIR":PATH="${UNITTESTPP_DIR}/install/lib/cmake/UnitTest++" \ # If necessary
        -D ZLIB_LIBRARY="${ZLIB_DIR}/libz.a" \                                      # If necessary
        -D ZLIB_INCLUDE_DIR="${ZLIB_DIR}"" \                                        # If necessary
        -D "AWSSDK_DIR":PATH="${AWS_DIR}/lib64/cmake/AWSSDK" \                      # For S3 support, you need to do this.
        -D "aws-cpp-sdk-core_DIR":PATH="${AWS_DIR}/lib64/cmake/aws-cpp-sdk-core" \
        -D "aws-c-event-stream_DIR":PATH="${AWS_DIR}/lib64/aws-c-event-stream/cmake" \
        -D "aws-c-common_DIR":PATH="${AWS_DIR}/lib64/aws-c-common/cmake" \
        -D "aws-checksums_DIR":PATH="${AWS_DIR}/lib64/aws-checksums/cmake" \
        -D "aws-cpp-sdk-s3_DIR":PATH="${AWS_DIR}/lib64/cmake/aws-cpp-sdk-s3" \                
        -DCMAKE_BUILD_TYPE=Release                                             
        ..
    cd ..
    cmake --build ${MODE} --target all -- -j 2                                
    ```

    **<span style="color:ffffa0;">
    Note, that the -D flags for the includes are only necessary, if you
    installed the libraries manually. If they are already installed on your 
    system or (e.g. for UnitTest++) you installed them to the system folders
    or if you use the Conda environment, you can omit these flags.
    </span>**

3. To clean the build directory use:

    ``` Bash
    cmake --build build --target clean -- -j 2
    ```

4. To run the tests, run the test binary like:

    ``` Bash
    (cd build/test && ./testapp)
    ```

5. If you want, you can add the release or debug directory to your PATH
    variable. E.g. in your local .bashrc file add the following:

    ``` bash
    # This assumes, that you cloned the repo to ~/Projects/FastqIndEx and 
    # created the release sub directory like described above.
    export PATH=~/Projects/FastqIndEx/release/src:$PATH
    ```

## Links

* [AWS](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup.html)
* [CMake](https://cmake.org/)
* [Miniconda](https://docs.conda.io/en/latest/miniconda.html)
* [UnitTest++](https://github.com/unittest-cpp/unittest-cpp)
