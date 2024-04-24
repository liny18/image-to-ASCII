#!/bin/bash

NUM=("1" "2" "3" "4" "5" "6" "12")
NODES=("1" "1" "1" "1" "1" "1" "2")
NTASKS=("1" "2" "3" "4" "5" "6" "6")
GPUS=("1" "2" "3" "4" "5" "6" "6")
OUTPUT=("1_rank_data.txt" "2_rank_data.txt" "3_rank_data.txt" "4_rank_data.txt" "5_rank_data.txt" "6_rank_data.txt" "12_rank_data.txt")
RANK=("1" "2" "3" "4" "5" "6" "12")
WIDTH=("10000" "10000" "10000" "10000" "10000" "10000" "10000")

num_configs=${#NODES[@]}

module load xl_r spectrum-mpi cuda/11.2
