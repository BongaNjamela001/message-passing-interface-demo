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

// Function to divide an nxn matrix into m matrices of 4 columns each
void divideIntoMatrices(int n, int m, double (*matrix)[n], double (*matrices)[m][n]) {
    int columns_per_matrix = 4;
    int padded_columns = (n % columns_per_matrix == 0) ? n : (n + (columns_per_matrix - n % columns_per_matrix));
    int columns_per_process = padded_columns / m;

    for (int i = 0; i < m; i++) {
        int start_column = i * columns_per_process;
        for (int j = 0; j < columns_per_matrix; j++) {
            if (start_column + j < n) {
                for (int k = 0; k < n; k++) {
                    matrices[i][j][k] = matrix[k][start_column + j];
                }
            } else {
                for (int k = 0; k < n; k++) {
                    matrices[i][j][k] = 0.0; // Padding with zeroes
                }
            }
        }
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

// Function to scatter and sort m matrices
void scatterAndSortMatrices(int m, int n, double (*matrices)[m][n], double (*sortedMatrices)[m][n]) {
    int number_of_process, rank_of_process;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_of_process);

    int columns_per_matrix = 4;
    int padded_columns = (n % columns_per_matrix == 0) ? n : (n + (columns_per_matrix - n % columns_per_matrix));
    int columns_per_process = padded_columns / m;

    double matrixCol[columns_per_matrix][n];
    double sortedMatrixCol[columns_per_matrix][n];

    for (int i = 0; i < m; i++) {
        // Scatter the matrix
        MPI_Scatter(matrices[i], n, MPI_DOUBLE, matrixCol, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Sort each column
        for (int j = 0; j < columns_per_matrix; j++) {
            bubblesort(n, matrixCol[j]);
        }

        // Gather the sorted columns
        MPI_Gather(matrixCol, n, MPI_DOUBLE, sortedMatrixCol, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if (rank_of_process == 0) {
            // Store the sorted columns
            for (int j = 0; j < columns_per_matrix; j++) {
                for (int k = 0; k < n; k++) {
                    sortedMatrices[i][j][k] = sortedMatrixCol[j][k];
                }
            }
        }
    }
}

// Function to combine sorted m matrices into an nxn matrix
void combineMatrices(int m, int n, double (*sortedMatrices)[m][n], double (*combinedMatrix)[n]) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < n; k++) {
                combinedMatrix[j + i * 4][k] = sortedMatrices[i][j][k];
            }
        }
    }
}

void transposeMatrix(int n, double matrix[n][n]) {
    double temp;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            temp = matrix[i][j];
            matrix[i][j] = matrix[j][i];
            matrix[j][i] = temp;
        }
    }
}

int main(int argc, char* argv[]) {
    const char* filename = "10x10.csv";
    int n = 10; // Change this to your desired matrix size
    int m = 3; // Change this to the number of matrices you want
    int number_of_process, rank_of_process;

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

    // Load the matrix from the file on root process
    if (rank_of_process == 0) {
        loadMatrixArray(filename, n, matrix);
        printMatrixArray(n, matrix, 0);
    }
    // transposeMatrix(n, matrix);
    // Barrier to synchronize processes
    MPI_Barrier(MPI_COMM_WORLD);

    // Divide the matrix into m matrices
    double (*matrices)[m][n] = malloc(m * sizeof(double[m][n]));
    double (*sortedMatrices)[m][n] = malloc(m * sizeof(double[m][n]));
    divideIntoMatrices(n, m, matrix, matrices);

    // Scatter and sort each matrix
    scatterAndSortMatrices(m, n, matrices, sortedMatrices);
    MPI_Barrier(MPI_COMM_WORLD);

    // Recombine sorted matrices into an nxn matrix
    double (*combinedMatrix)[n] = malloc(n * sizeof(double[n]));
    combineMatrices(m, n, sortedMatrices, combinedMatrix);
    transposeMatrix(n, combinedMatrix);
    // Print the combined matrix on root process
    if (rank_of_process == 0) {
        printMatrixArray(n, combinedMatrix, 1);
    }

    // Free allocated memory
    free(matrix);
    free(matrices);
    free(sortedMatrices);
    free(combinedMatrix);

    MPI_Finalize();

    return 0;
}
