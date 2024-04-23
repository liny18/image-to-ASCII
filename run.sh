#!/bin/bash

source ./template.sh

mkdir -p "performance_data"
mkdir -p "shell_scripts"

for (( i=0; i<num_configs; i++ )); do
    job_script="shell_scripts/job_script_${NUM[i]}.sh"

    cat > "$job_script" << EOF
#!/bin/bash
#SBATCH --job-name=project${NUM[i]}
#SBATCH --nodes=${NODES[i]}
#SBATCH --ntasks-per-node=${NTASKS[i]}
#SBATCH --gres=gpu:${GPUS[i]}
#SBATCH --time=00:03:00
#SBATCH --partition=el8
#SBATCH --output=performance_data/${OUTPUT[i]}

module load xl_r spectrum-mpi cuda/11.2

mpirun -np ${RANK[i]} ./image_to_ascii -i images/hwoarang.png -w ${WIDTH[i]} -c -t 256
EOF

    sbatch "$job_script"
done
