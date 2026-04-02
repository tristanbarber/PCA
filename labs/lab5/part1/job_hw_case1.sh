#!/bin/bash
#SBATCH --mail-type=fail
#SBATCH --mail-user=tbarber@ufl.edu
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=500mb
#SBATCH -t 00:05:00
#SBATCH -o outfile
#SBATCH -e errfile
srun --mpi=$HPC_PMIX ./cmtbonebe 100 5 4 4 4 2 2 2
