#!/bin/bash
#SBATCH --job-name=project2
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=2
#SBATCH --gres=gpu:2
#SBATCH --time=00:03:00
#SBATCH --partition=el8
#SBATCH --output=performance_data/2_rank_data.txt

module load xl_r spectrum-mpi cuda/11.2

mpirun -np 2 ./image_to_ascii -i images/hwoarang.png -w 10000 -c -t 256
