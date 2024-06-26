#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install wget

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
mamba create -y -p $deps_dir cxx-compiler cmake libboost-devel ninja lcov
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

# GCC build.
cmake -G Ninja ../ -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_BUILD_TYPE=Debug \
    -DTANUKI_BUILD_TESTS=yes \
    -DTANUKI_BUILD_TUTORIALS=yes \
    -DTANUKI_WITH_BOOST_S11N=yes \
    -DCMAKE_CXX_FLAGS="--coverage"
ninja
ctest -V -j4

# Create lcov report.
# NOTE: this is apparently picked up automatically
# by the codecov action.
lcov --capture --directory . --output-file coverage.info

set +e
set +x
