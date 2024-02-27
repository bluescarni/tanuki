#!/usr/bin/env bash

# Echo each command.
set -x

# Exit on error.
set -e

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
    -DTANUKI_BUILD_TESTS=yes \
    -DTANUKI_BUILD_TUTORIALS=yes \
    -DTANUKI_WITH_BOOST_S11N=yes
make -j4
ctest . -j4

set +e
set +x
