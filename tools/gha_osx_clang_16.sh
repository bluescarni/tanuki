#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-x86_64.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
mamba create -y -q -p $deps_dir 'clangxx=16.*' cmake libboost-devel ninja
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

cmake -G Ninja ../ -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Debug \
    -DTANUKI_BUILD_TESTS=yes \
    -DTANUKI_BUILD_TUTORIALS=yes \
    -DTANUKI_WITH_BOOST_S11N=yes
ninja
ctest -V -j4

set +e
set +x
