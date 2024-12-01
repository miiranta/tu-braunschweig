#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <mpi.h>

#define N 16 // matrix size

// function to print a matrix
void print_matrix(int *matrix, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      printf("%d ", matrix[i * cols + j]);
    }
    printf("\n");
  }
}

int main(int argc, char **argv) {

  MPI_Init(&argc,&argv);
  int number_of_processes = 0;
  int my_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&number_of_processes);

  int rows_per_proc = N/number_of_processes;
 

  int *A, *B, *C; // matrices
  // allocate memory for matrices
  A = (int *)malloc(N * N * sizeof(int));
  B = (int *)malloc(N * rows_per_proc * sizeof(int));
  C = (int *)malloc(N * rows_per_proc * sizeof(int));
  int* iniB = (int *)malloc(N * N * sizeof(int));

  //ZERO
  if(my_rank == 0)
  {
    // initialize matrices with random values
    srand(0); // seed for reproducibility
    for (int i = 0; i < N * N; i++) {
      A[i] = i;
      iniB[i] = i;
    }
    printf("Matrix A:\n");
    print_matrix(A, N, N);
    
  }

  //BROADCAST A
  int root = 0;
  MPI_Bcast(A, N*N, MPI_INT, root, MPI_COMM_WORLD);

  //SCATTER B
  MPI_Scatter(iniB, N * rows_per_proc, MPI_INT, B, N * rows_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
  printf("Matrix B (rank %d):\n", my_rank);
  print_matrix(B, 1, N * rows_per_proc);


  const auto start_chrono = std::chrono::high_resolution_clock::now();

  // perform matrix multiplication on local matrices
  for (int i=0; i<N * rows_per_proc;i++ ){
    C[i] = 0;
    for (int j=0; j<N * rows_per_proc;j++ ){
      C[i] = C[i] + B[j]*A[j + ];
    }
  }

  const auto finish_chrono = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> diff = finish_chrono - start_chrono;

  
  //GATHER
  int* resultC = (int *)malloc(N * N * sizeof(int));
  MPI_Gather(C, N * rows_per_proc, MPI_INT, resultC, N * rows_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

  //print the result matrix
  if(my_rank == 0)
  {
    printf("Matrix C:\n");
    print_matrix(resultC, N, N);
  }
  

  // free memory and finalize MPI
  free(A);
  free(B);
  free(C);
  free(iniB);
  free(resultC);
  MPI_Finalize();
  std::cout << "Runtime: " << diff.count() << " s. " << std::endl;

  return 0;
}