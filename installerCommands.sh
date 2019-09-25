#
# Copyright (c) 2019 DKFZ - ODCF
#
# Distributed under the MIT License (license terms are at https://github.com/dkfz-odcf/FastqInDex/blob/master/LICENSE.txt).
#

# Various tryInstaller commands for createBuildDir.sh

function _run() {
  local cmd=$1
  [[ $dry == false ]] && $cmd || echo $cmd
}

function tryInstallCMake() {
  (
    if [[ ! -d cmake-3.13.4 ]]; then
      wget https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4.tar.gz
      tar -xvzf cmake-3.13.4.tar.gz
      rm cmake-3.13.4.tar.gz
    else
      echo "CMake is already installed, won't do it again."
    fi
    echo "Please make sure to use this cmake binary ('${PWD}/cmake-3.13.4/bin/cmake') " \
      "for your later builds (e.g. by putting it to your path)."
  )
  export PATH=$PWD/cmake-3.13.4/bin:$PATH
}

function tryInstallZLib() {
  (
    if [[ ! -d zlib-1.2.11 ]]; then
      wget https://www.zlib.net/zlib-1.2.11.tar.gz
      tar -xvzf zlib-1.2.11.tar.gz
      #     Configure and build afterwards
      (cd zlib-1.2.11 && ./configure && make)
      rm zlib-1.2.11.tar.gz
    else
      echo "ZLib is already installed, won't do it again."
    fi
  )
}

function tryInstallUnittestCPP() {
  (
    if [[ ! -d "unittest-cpp" ]]; then
      git clone https://github.com/unittest-cpp/unittest-cpp.git
      cd unittest-cpp
      mkdir builds
      cd builds
      cmake -G "Unix Makefiles" -D CMAKE_INSTALL_PREFIX=/custom/lib/path ..
      cmake --build . --target all
      cmake --build . --target tryInstall
    else
      echo "UnittestCPP is already installed, won't do it again."
    fi
  )
}

function tryInstallSimpleIni() {
  (
    if [[ ! -d simpleini_4.17 ]]; then
      git clone -b '4.17' --single-branch --depth 1 https://github.com/brofield/simpleini.git simpleini_4.17
    else
      echo "SimpleIni is already installed, won't do it again."
    fi
  )
}

function tryInstallTClap() {
  (
    if [[ ! -d tclap-1.2.1 ]]; then
      wget https://vorboss.dl.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz
      tar -xvzf tclap-1.2.1.tar.gz && rm tclap-1.2.1.tar.gz
    else
      echo "TClap is already installed, won't do it again."
    fi
  )
}
