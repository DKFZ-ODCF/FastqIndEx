#
# Copyright (c) 2019 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#

# Build script to:
# - Create a build directory, if necessary
# - Make the target
# - Run the tests
# The script will try to use the conda environmant, if possible.

set -eu
source installerCommands.sh

# Determine the type of the build.
type="Release"
[[ "${1-release}" == "debug" ]] && declare -x type=Debug
[[ "${2-false}" == "dry" ]] && declare -x dry=true || declare -x dry=false

declare -x buildFolder="cmake-build-${type,,}"

echo "The type of the build configuration is '${type}'"
echo "Using the build folder '${buildFolder}'"
if [[ "$dry" == true ]]; then
  echo "Dry run is active, will only display commands instead of running them."
  set -xv
fi

# Check if conda is available
declare -x condaExists=$(
  which conda &>/dev/null
  [[ $? != 0 ]] && echo false || echo true
)

# Check if the conda FastqIndEx environment is available
if [[ $condaExists == true ]]; then
  echo "A conda installation is present."
  fastqEnvExists=$(
    conda env list | grep "FastqIn2dEx "
    [[ $? == 1 ]] && echo false || echo true
  )
  if [[ $fastqEnvExists == true ]]; then
    echo "The conda 'FastqIndEx' environment exists. I will activate it."
    conda activate "FastqIndEx"
  else
    echo "The conda environment 'FastqIndEx' does not exist. Shall I install it?"
    declare -x installConda=false
    select yn in "Yes" "No"; do
      case $yn in
      Yes)
        installConda=true
        break
        ;;
      No)
        condaExists=false
        break
        ;;
      esac
    done
    if [[ $installConda == true ]]; then
      _run "conda env create -f=env/environment.yml"
    fi
  fi
fi

# Change to parent directory for the next steps.
cd ..

# Either use conda now or manual installation
if [[ $condaExists == true ]]; then
  echo "Will use the existing conda environment."
  _run "conda activate 'FastqIndEx'"

  export CONDA_LIB=${CONDA_PREFIX}/lib
  export CONDA_LIB64=${CONDA_PREFIX}/lib64
  export CONDA_INCLUDE=${CONDA_PREFIX}/include

  export ZLIB_LIBRARY="${CONDA_PREFIX}/lib/libz.a"
  export ZLIB_INCLUDE_DIR="${CONDA_PREFIX}/include"
  export TCLAP_INCLUDE_DIR="${CONDA_PREFIX}/include/tclap"
else
  echo "conda is not present or the 'FastqIndEx' environment not installed, will build manually."

  # Check/Resolve dependencies not available in conda, keep everything in a separate process.
  _run "tryInstallCMake"
  _run "tryInstallZLib"
  _run "tryInstallUnittestCPP"
  _run "tryInstallSimpleIni"
  _run "tryInstallTClap"

  export ZLIB_LIBRARY="${CONDA_PREFIX}/lib/libz.a"
  export ZLIB_INCLUDE_DIR="${CONDA_PREFIX}/include"
  export SIMPLEINI_INCLUDE_DIR ../simpleini_4.17
  export TCLAP_INCLUDE_DIR="../tclap-1.2.1/include"
    -D "AWSSDK_DIR:PATH=${AWS_DIR}/lib64/cmake/AWSSDK" \                      # For S3 support, you need to do this.
    -D "aws-cpp-sdk-core_DIR:PATH=${AWS_DIR}/lib64/cmake/aws-cpp-sdk-core" \
    -D "aws-c-event-stream_DIR:PATH=${AWS_DIR}/lib64/aws-c-event-stream/cmake" \
    -D "aws-c-common_DIR:PATH=${AWS_DIR}/lib64/aws-c-common/cmake" \
    -D "aws-checksums_DIR:PATH=${AWS_DIR}/lib64/aws-checksums/cmake" \
    -D "aws-cpp-sdk-s3_DIR:PATH=${AWS_DIR}/lib64/cmake/aws-cpp-sdk-s3" \

fi

# And back.
cd FastqIndEx

[[ ! -d $buildFolder ]] && mkdir $buildFolder

cd $buildFolder



#cmake -D CMAKE_BUILD_TYPE=${type} -D BUILD_ONLY="s3;config;transfer" -D BUILD_SHARED_LIBS=ON  -D OPENSSL_ROOT_DIR=${CONDA_DIR}  -D OPENSSL_INCLUDE_DIR=${CONDA_INCLUDE} OPENSSL_LIBRARIES=${CONDA_LIB}  -D ZLIB_INCLUDE_DIR=${CONDA_INCLUDE} -DZLIB_LIBRARY=${CONDA_LIB}/libz.a -D CURL_INCLUDE_DIR=${CONDA_INCLUDE} -DCURL_LIBRARY=${CONDA_LIB}/libcurl.so  -D CMAKE_INSTALL_PREFIX=$PWD/../install_shared -D ENABLE_TESTING=OFF ..
#    cmake -D CMAKE_BUILD_TYPE=Debug -D BUILD_ONLY="s3;config;transfer" -D BUILD_SHARED_LIBS=OFF  -D OPENSSL_ROOT_DIR=${CONDA_DIR}  -D OPENSSL_INCLUDE_DIR=${CONDA_INCLUDE} OPENSSL_LIBRARIES=${CONDA_LIB}  -D ZLIB_INCLUDE_DIR=${CONDA_INCLUDE} -DZLIB_LIBRARY=${CONDA_LIB}/libz.a -D CURL_INCLUDE_DIR=${CONDA_INCLUDE} -DCURL_LIBRARY=${CONDA_LIB}/libcurl.so  -D CMAKE_INSTALL_PREFIX=$PWD/../install_shared -D ENABLE_TESTING=OFF ..
#
#
#    export AWS_DIR="/path/to/aws-sdk-cpp/install_shared"
#    export UNITTESTPP_DIR="/path/to/unittest-cpp"
#    export ZLIB_DIR="/path/to/zlib-1.2.11"                                    # If necessary
#

cmake -G "Unix Makefiles" \
    -D "UnitTest++_DIR:PATH=${UNITTESTPP_DIR}/install/lib/cmake/UnitTest++" \ # If necessary
    -D ZLIB_LIBRARY="${ZLIB_DIR}/libz.a" \                                      # If necessary
    -D ZLIB_INCLUDE_DIR="${ZLIB_DIR}" \                                         # If necessary
    -D "AWSSDK_DIR:PATH=${AWS_DIR}/lib64/cmake/AWSSDK" \                      # For S3 support, you need to do this.
    -D "aws-cpp-sdk-core_DIR:PATH=${AWS_DIR}/lib64/cmake/aws-cpp-sdk-core" \
    -D "aws-c-event-stream_DIR:PATH=${AWS_DIR}/lib64/aws-c-event-stream/cmake" \
    -D "aws-c-common_DIR:PATH=${AWS_DIR}/lib64/aws-c-common/cmake" \
    -D "aws-checksums_DIR:PATH=${AWS_DIR}/lib64/aws-checksums/cmake" \
    -D "aws-cpp-sdk-s3_DIR:PATH=${AWS_DIR}/lib64/cmake/aws-cpp-sdk-s3" \
    -DCMAKE_BUILD_TYPE=${type}
    ..
#cd ..
#cmake --build ${type} --target all -- -j 2
