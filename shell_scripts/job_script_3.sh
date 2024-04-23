#!/bin/bash
#SBATCH --job-name=project3
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=3
#SBATCH --gres=gpu:3
#SBATCH --time=00:03:00
#SBATCH --partition=el8
#SBATCH --output=performance_data/3_rank_data.txt

module load xl_r spectrum-mpi cuda/11.2

mpirun -np 3 ./image_to_ascii -i images/hwoarang.png -w 10000 -c -t 256
