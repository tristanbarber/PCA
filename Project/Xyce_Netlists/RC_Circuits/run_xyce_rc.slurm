#!/bin/bash
#SBATCH --job-name=xyce_sim
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16gb
#SBATCH --time=01:00:00
#SBATCH -o xyce_sim_rc.out
#SBATCH -e xyce_sim_rc.err

source ../../build_configs/modules/load_env.sh

XYCE=~/EEL6763/xyce_project/Xyce/build/src/Xyce
NETLIST=rc_tree_xyce.cir

mpirun -np $SLURM_NTASKS $XYCE $NETLIST
