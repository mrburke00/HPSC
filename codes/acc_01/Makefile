# -----------------------------------------------------
# On local Ubuntu
# -----------------------------------------------------
#
# Be sure to use MPI compiler wrappers.
# Whenever compiling an MPI program, you should use the MPI wrappers:
#
#    C - mpicc
#    C++ - mpiCC, mpicxx, mpic++
#    FORTRAN - mpifort, mpif77, mpif90
#
# These wrappers do all of the dirty work for you of making sure that
# all of the appropriate compiler flags, libraries, include directories,
# library directories, etc. are included when you compile your program.


# -----------------------------------------------------
# On Summit:
# -----------------------------------------------------
#
# module load intel
# module load impi

# -----------------------------------------------------
# Set compiler based on platform
# -----------------------------------------------------

# Default compiler is Intel MPI compiler

COMPILER := mpicc
UBUNTU   := ubuntu

# Change if on ubuntu

ifeq ($(strip $(SESSION)),$(UBUNTU))
COMPILER := mpicxx
endif

# -----------------------------------------------------
# Make esPIC
# -----------------------------------------------------


main: main.cpp Makefile
	pgc++ -O3 -acc main.cpp -o main -std=c++11


#	g++ -O3 -fopenmp -fopenacc main.cpp -o main -std=c++11
#	$(COMPILER) -fopenmp -fopenacc main.cpp -o main -std=c++11






