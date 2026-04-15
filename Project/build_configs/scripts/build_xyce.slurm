#!/bin/bash
#SBATCH --job-name=xyce_build
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --partition=hpg-dev
#SBATCH --cpus-per-task=8
#SBATCH --mem=16gb
#SBATCH --time=04:00:00
#SBATCH -o xyce.out
#SBATCH -e xyce.err

source ../modules/load_env.sh

cd ~/EEL6763/xyce_project/Xyce/build
cmake --build . -j 8
