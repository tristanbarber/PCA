#!/bin/bash

source ../modules/load_env.sh

cd ~/xyce_deps/build/suitesparse
rm -rf *

cmake \
  -D CMAKE_BUILD_TYPE=Release \
  -D CMAKE_INSTALL_PREFIX=$HOME/xyce_deps/install/suitesparse \
  -D BUILD_SHARED_LIBS=ON \
  -D SUITESPARSE_ENABLE_PROJECTS="suitesparse_config;amd;colamd;btf;klu" \
  -D BLAS_LIBRARIES=/lib64/libopenblaso.so.0 \
  -D LAPACK_LIBRARIES=/lib64/libopenblaso.so.0 \
  ~/xyce_deps/src/SuiteSparse-7.8.3
