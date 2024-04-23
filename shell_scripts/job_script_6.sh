#!/bin/bash
#SBATCH --job-name=project6
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=6
#SBATCH --gres=gpu:6
#SBATCH --time=00:03:00
#SBATCH --partition=el8
#SBATCH --output=performance_data/6_rank_data.txt

module load xl_r spectrum-mpi cuda/11.2

mpirun -np 6 ./image_to_ascii -i images/hwoarang.png -w 10000 -c -t 256
