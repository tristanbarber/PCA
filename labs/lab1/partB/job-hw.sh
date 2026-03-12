#!/bin/bash
#SBATCH --job-name=lab1B
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=tbarber@ufl.edu
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1000mb
#SBATCH -t 00:05:00
#SBATCH -o myoutput
#SBATCH -e myerr
srun --mpi=$HPC_PMIX ./a.out -10 10 100000
