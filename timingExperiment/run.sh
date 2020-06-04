#!/bin/bash

#SBATCH --job-name=timingKernelConvolution
#SBATCH --partition=cpu
#SBATCH --cpus-per-task=28
#SBATCH --mem-per-cpu=8G
#SBATCH --nodes=1
#SBATCH --output=kernelConvOutput.out
#SBATCH --time=120:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=ssingh5@scu.edu
#

export JSLOG=1
export INWAVE=1
export OMP_PLACES=cores
export OMP_PROC_BIND=spread


for THREADNUM in 1 2 4 8 12 14 16 20 24 28
do
	export OMP_NUM_THREADS=$THREADNUM
	./openMP

done

