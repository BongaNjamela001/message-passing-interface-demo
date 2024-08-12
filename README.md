 
# MPI Bubble Sort Performance Investigation

## Overview

This project investigates the performance of a bubble-sort algorithm implemented using MPI (Message Passing Interface) compared to a MATLAB implementation. The MATLAB version employs a serial approach, while the MPI version utilizes parallelization to distribute the workload across multiple processors. The investigation primarily focuses on analyzing the speedup achieved by the MPI program as the size of an input n x n matrix increases. Additionally, the study explores how the number of processors in the communicator impacts performance.

## Project Structure

sortingmpi.c (MPI Implementation)

A C program implementing the bubble-sort algorithm using MPI. It distributes matrix data across multiple processors, sorts the data in parallel, and gathers the results. The MPI program is executed on a single CPU using a scatter and gather memory partitioning scheme.
bubbleSort.m (MATLAB Implementation)

A serial bubble-sort algorithm written in MATLAB, used as a baseline for performance comparison. The matrix used for sorting is generated using MATLAB's random number generation function and stored in CSV files.
generateCSV.m (Data Generation)

A MATLAB script to generate random n x n matrices and store them in CSV files, which are then used as input for both the MPI and MATLAB implementations.
Implementation Details
MPI Program

## Memory Distribution and Sorting

The MPI program distributes an n x n matrix among processors using the MPI_Scatter function. Each processor sorts its portion of the matrix in parallel using the bubble-sort algorithm, and the results are then gathered using MPI_Gather.
Synchronization and Timing

The program ensures synchronization between processes using MPI_Barrier to prevent data contention and race conditions. Execution time is measured using MPI_Wtime, allowing for a comparison of the MPI program's performance against the MATLAB implementation.
Running the MPI Program

The program can be executed with varying numbers of processors to analyze performance. Example commands to run the program are:

/usr/bin/mpicc sortingmpi.c -o sortingmpi
/usr/bin/mpirun -np 4 ./sortingmpi
/usr/bin/mpirun -np 3 ./sortingmpi
/usr/bin/mpirun -np 2 ./sortingmpi

## MATLAB Program

### Serial Implementation
The MATLAB program processes the entire matrix on a single core, sorting columns serially using the bubble-sort algorithm. The execution time of this program serves as the baseline for evaluating the performance improvements gained through MPI parallelization.

## Results
The MPI implementation demonstrates significant performance improvements as the matrix size increases, achieving a maximum average speedup of 20x compared to the MATLAB version. However, the performance benefit varies with the number of processors used, with peak performance observed when using 2 processors.

## Conclusion
The investigation highlights the effectiveness of MPI parallelization in improving the performance of sorting algorithms compared to serial implementations like MATLAB. MPI offers better scalability, resource utilization, and performance optimization, making it a suitable choice for parallelizing computationally intensive algorithms on modern hardware architectures.

