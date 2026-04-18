#!/bin/bash
#SBATCH --job-name=xyce_regression
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16gb
#SBATCH --time=01:00:00
#SBATCH -o xyce_sim.out
#SBATCH -e xyce_sim.err

source ../build_configs/modules/load_env.sh

cd ~/EEL6763/xyce_testing

ctest -I 1,3265 \
  -L "parallel" \
  -LE "nonfree" \
  -j $SLURM_NTASKS \
  -R "NEURON|SENS|TL1X|CircuitSim90|POWER_GRID|SAMPLING|BSIM|MOS2_LARGE|PDE|SANDLER" \
  -E "ac-sens-step-tecplot|ac-sens-tecplot|mpde-csv|mpde-tecplot|sens-step-tecplot|sens-tecplot|sens-tranadjoint-overrideRaw|diode\.cir\.sh|invertbjt" \
  | tee parallel_sweep_results.txt
