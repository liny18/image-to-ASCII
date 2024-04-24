#!/bin/bash
#SBATCH --job-name=project12
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=6
#SBATCH --gres=gpu:6
#SBATCH --time=00:03:00
#SBATCH --partition=el8
#SBATCH --output=performance_data/12_rank_data.txt

module load xl_r spectrum-mpi cuda/11.2

mpirun -np 12 ./image_to_ascii -i images/hwoarang.png -w 10000 -c -t 256
