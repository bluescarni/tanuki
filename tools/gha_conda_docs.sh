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
mamba create -y -q -p $deps_dir cmake python=3.11 'sphinx=7.*' 'sphinx-book-theme=1.*' make
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

cmake ../
cd ../doc
make html linkcheck

set +e
set +x
