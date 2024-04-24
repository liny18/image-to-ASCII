#!/bin/bash
#SBATCH --job-name=project1
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --gres=gpu:1
#SBATCH --time=00:03:00
#SBATCH --partition=el8
#SBATCH --output=performance_data/1_rank_data.txt

module load xl_r spectrum-mpi cuda/11.2

mpirun -np 1 ./image_to_ascii -i images/hwoarang.png -w 10000 -c -t 256
