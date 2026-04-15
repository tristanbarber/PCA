#!/bin/bash

source ../modules/load_env.sh

cd ~/EEL6763/xyce_project/Xyce
rm -rf build
mkdir build
cd build

cmake \
  -D Trilinos_ROOT=$HOME/xyce_deps/install/trilinos-14.4-openmpi507 \
  -D CMAKE_C_COMPILER=mpicc \
  -D CMAKE_CXX_COMPILER=mpicxx \
  -D CMAKE_Fortran_COMPILER=mpifort \
  ..
