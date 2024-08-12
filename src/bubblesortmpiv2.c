#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_LINE_LENGTH 1048576

// Function to read CSV file into an nxn matrix
void loadMatrixArray(const char* filename, int n, double matrix[n][n]) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Unable to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    char line[MAX_LINE_LENGTH];
    int row = 0;
    while (fgets(line, sizeof(line), file) && row < n) {
        char* token = strtok(line, ",");
        int col = 0;
        while (token != NULL && col < n) {
            matrix[row][col++] = atof(token);
            token = strtok(NULL, ",");
        }
        row++;
    }
    fclose(file);
}

// Function to print the loaded array
void printMatrixArray(int n, double matrix[n][n], int isSorted) {
    if(isSorted == 1) {
        printf("Sorted matrix:\n");
    }
    else {
        printf("Original unsorted matrix:\n");
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%.3f ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void columnize(int n, double matrix[n][n], double** matrixCols) {
    for(int j = 0; j < n; j++) {
        for(int i = 0; i < n; i++) {
            matrixCols[j][i] = matrix[i][j]; 
            // printf("Matrix col entry %i: %.3f \n", i, matrixCols[j][i]);
        }
    }
}

void swapEntries(double* matrixCol, int j) {
    if (matrixCol[j] > matrixCol[j+1]) {
        double temp = matrixCol[j];
        matrixCol[j] = matrixCol[j+1];
        matrixCol[j+1] = temp;
    }
}

// Function to perform bubble sort on an array
void bubblesort(int n, double* arr) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                // Swap elements
                double temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    const char* filename = "10x10.csv";
    int n = 10; // Change this to your desired matrix size
    int number_of_process, rank_of_process;
    double time_taken;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        printf("Program terminated. Could not create MPI program.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_of_process);

    // Allocate memory for the matrix
    double (*matrix)[n] = malloc(n * sizeof(double[n]));
    if (!matrix) {
        printf("Error: Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    // Load the matrix from the file
    if (rank_of_process == 0) {
        loadMatrixArray(filename, n, matrix);
        printMatrixArray(n, matrix, 0);
    }

    // Broadcast the matrix to all processes
    MPI_Bcast(matrix, n*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Allocate memory for the columns of the matrix
    double* matrixCols = malloc(n * sizeof(double));
    if (!matrixCols) {
        printf("Error: Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    // Scatter the columns of the matrix to different processes
    MPI_Scatter(matrix, n, MPI_DOUBLE, matrixCols, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Sort the columns independently on each process
    bubblesort(n, matrixCols);

    // Gather the sorted columns back to the root process
    MPI_Gather(matrixCols, n, MPI_DOUBLE, matrix, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Print the sorted matrix on the root process
    if (rank_of_process == 0) {
        printMatrixArray(n, matrix, 1);
    }

    // Free allocated memory
    free(matrix);
    free(matrixCols);

    // Finalize MPI
    MPI_Finalize();

    return 0;
}
