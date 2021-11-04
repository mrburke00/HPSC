#!/bin/bash

# -
# |
# | This is a batch script for running a MPI parallel job on Summit
# |
# | (o) To submit this job, enter:  sbatch --export=CODE='/home/scru5660/HPSC/codes/fd_mpi/src' ex_01.bat 
# |
# | (o) To check the status of this job, enter: squeue -u <username>
# |
# -

# -
# |
# | Part 1: Directives
# |
# -

#SBATCH --nodes=1
#SBATCH --ntasks=1 --cpus-per-task 16
#SBATCH --time=00:20:00
#SBATCH --partition=shas-testing
#SBATCH --output=ex01-%j.out

# -
# |
# | Part 2: Loading software
# |
# -

module purge
module load intel
module load impi 

# -
# |
# | Part 3: User scripting
# |
# -

echo "=="
echo "||"
echo "|| Begin Execution of fd in slurm batch script."
echo "||"
echo "=="

mpirun -n 1 ../src/./solvers -nPEx 1 -nPEy 1 -nCellx 60 -nCelly 60 -solver cg -nl nr | tee tmp



echo "=="
echo "||"
echo "|| Execution of solver in slurm batch script complete."
echo "||"
echo "=="








