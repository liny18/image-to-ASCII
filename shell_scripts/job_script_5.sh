#!/bin/bash
#SBATCH --job-name=project5
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=5
#SBATCH --gres=gpu:5
#SBATCH --time=00:03:00
#SBATCH --partition=el8
#SBATCH --output=performance_data/5_rank_data.txt

module load xl_r spectrum-mpi cuda/11.2

mpirun -np 5 ./image_to_ascii -i images/hwoarang.png -w 10000 -c -t 256
