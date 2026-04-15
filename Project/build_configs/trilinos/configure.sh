#!/bin/bash

source ../modules/load_env.sh

cd ~/xyce_deps/build/trilinos-14.4
rm -rf *

cmake \
  -C ~/EEL6763/xyce_project/Xyce/cmake/trilinos/trilinos-MPI-base.cmake \
  -D CMAKE_INSTALL_PREFIX=$HOME/xyce_deps/install/trilinos-14.4-openmpi507 \
  -D CMAKE_C_COMPILER=mpicc \
  -D CMAKE_CXX_COMPILER=mpicxx \
  -D CMAKE_Fortran_COMPILER=mpifort \
  -D TPL_ENABLE_BLAS=ON \
  -D TPL_ENABLE_LAPACK=ON \
  -D TPL_BLAS_LIBRARIES=/lib64/libopenblaso.so.0 \
  -D TPL_LAPACK_LIBRARIES=/lib64/libopenblaso.so.0 \
  -D TPL_ENABLE_AMD=ON \
  -D TPL_AMD_LIBRARIES=$HOME/xyce_deps/install/suitesparse/lib64/libamd.so \
  -D TPL_AMD_INCLUDE_DIRS=$HOME/xyce_deps/install/suitesparse/include/suitesparse \
  -D Trilinos_ENABLE_EpetraExt=ON \
  -D EpetraExt_BUILD_EXPERIMENTAL=ON \
  -D EpetraExt_BUILD_GRAPH_REORDERINGS=ON \
  -D EpetraExt_BUILD_BTF=ON \
  -D EpetraExt_ENABLE_AMD=ON \
  -D Trilinos_ENABLE_Amesos=ON \
  -D Amesos_ENABLE_KLU=ON \
  -D Trilinos_ENABLE_NOX=ON \
  -D NOX_ENABLE_LOCA=ON \
  ~/xyce_deps/src/Trilinos-trilinos-release-14-4-0
