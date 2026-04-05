#!/bin/bash
#SBATCH --job-name=lab5_part2
#SBATCH --output=part2_out.txt
#SBATCH --error=part2_err.txt
#SBATCH --time=00:05:00
#SBATCH --partition=hpg-default
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

./klu_test
