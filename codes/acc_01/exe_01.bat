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
#SBATCH --ntasks=4
#SBATCH --time=00:20:00
#SBATCH --partition=sgpu
#SBATCH --output=slurm-%j.out

# -
# |
# | Part 2: Loading software
# |
# -

#module load gcc
module load pgi
 
# -
# |
# | Part 3: User scripting
# |
# -

echo "=="
echo "||"
echo "|| Begin Execution of ./main in slurm batch script."
echo "||"
echo "=="

./main -nCellx 100 -nCelly 100 -maxIter 10   > tty_100_100_10.out
#./main -nCellx 100 -nCelly 100 -maxIter 100  > tty_100_100_100.out
#./main -nCellx 100 -nCelly 100 -maxIter 1000 > tty_100_100_1000.out

echo "=="
echo "||"
echo "|| Execution of ./main in slurm batch script complete."
echo "||"
echo "=="








