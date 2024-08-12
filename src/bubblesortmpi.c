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

void bubblesort(int n, double* matrixCol) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            swapEntries(matrixCol, j);
        }
    }
}

// Function to sort columns of the matrix
void sortColumns(int n, double** matrixCols) {
    for (int j = 0; j < n; j++) {
        bubblesort(n, matrixCols[j]);
    }
}

int main(int argc, char* argv[]) {
    const char* filename = "10x10.csv";
    int n = 10; // Change this to your desired matrix size
    int number_of_process, rank_of_process;
    // int* data = NULL;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        printf("Program terminated. Could not create MPI program.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_of_process);
    // int chunk_size, own_chunk_size;
    // int* chunk;
    double time_taken;

    // Allocate memory for the matrix
    double (*matrix)[n] = malloc(n * sizeof(double[n]));
    double matrixCol[n];

    if (!matrix) {
        printf("Error: Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    // if (rank_of_process == 0) {
    loadMatrixArray(filename, n, matrix);
    printMatrixArray(n, matrix, 0);
    free(matrix);
    // }
    // Blocks all process until reach this point
    MPI_Barrier(MPI_COMM_WORLD);

    // MPI_Status status;

    MPI_Scatter(matrix, n, MPI_DOUBLE, matrixCol, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Sort the column
    bubblesort(n, matrixCol);
    MPI_Barrier(MPI_COMM_WORLD);

    // Gather the sorted columns back to the root process
    // double sortedMatrix[n][n];
    double (*sortedMatrix)[n] = malloc(n * sizeof(double[n]));

    MPI_Gather(matrixCol, n, MPI_DOUBLE, sortedMatrix, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // MPI_Barrier(MPI_COMM_WORLD);

    if (rank_of_process == 0) {
        printMatrixArray(n, sortedMatrix,1);
        free(sortedMatrix);
    }

    time_taken -= MPI_Wtime();

    MPI_Finalize();
    // Starts Timer 

    // // BroadCast the Size to all the process from root process
    // MPI_Bcast(&number_of_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // // Computing chunk size
    // chunk_size=(n%number_of_process == 0) ? (n/number_of_process):n/(number_of_process - 1);

    // // Calculating total size of chunk according to bits
    // chunk = (int*)malloc(chunk_size * sizeof(double));

    // // Scatter the chuck size data to all process
    // MPI_Scatter(matrix, chunk_size, MPI_INT, chunk,chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    // free(data);

    // matrix = NULL;

    // own_chunk_size = (number_of_elements
    //                   >= chunk_size * (rank_of_process + 1))
    //                      ? chunk_size
    //                      : (number_of_elements
    //                         - chunk_size * rank_of_process);

    // // Sorting array with quick sort for every
    // // chunk as called by process
    // quicksort(chunk, 0, own_chunk_size);

    return EXIT_SUCCESS;
}
