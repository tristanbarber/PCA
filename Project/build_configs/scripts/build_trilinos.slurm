#!/bin/bash
#SBATCH --job-name=trilinos_build
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --partition=hpg-dev
#SBATCH --cpus-per-task=8
#SBATCH --mem=16gb
#SBATCH --time=04:00:00
#SBATCH -o trilinos.out
#SBATCH -e trilinos.err

source ../modules/load_env.sh

cd ~/xyce_deps/build/trilinos-14.4
cmake --build . -j 8
cmake --build . -j 8 -t install
