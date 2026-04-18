#!/bin/bash

source ../modules/load_env.sh

cd ~/xyce_deps/src
git clone https://github.com/Xyce/Xyce_Regression.git

mkdir ~/EEL6763/xyce_testing
cd ~/EEL6763/xyce_testing

cmake \
  -DCMAKE_PREFIX_PATH=~/EEL6763/xyce_project/Xyce/build/src/ \
  ~/xyce_deps/src/Xyce_Regression