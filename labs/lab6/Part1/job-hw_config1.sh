#!/bin/bash
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --partition=hpg-turin
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=2gb
#SBATCH --gpus=1
#SBATCH -t 00:05:00
#SBATCH -o outfile_1
#SBATCH -e errfile_1

module load ufrc class/eel6763 cuda/12.4.1

ncu --print-summary per-kernel ./sobel_cuda input.txt 1

./sobel_cuda input.txt 1
