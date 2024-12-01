#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <immintrin.h>

#include <cuda_runtime.h>

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

__global__ void matrixMultiplicationKernel(int* A, int* B, int* C)
{

  int ROW = blockIdx.y*blockDim.y+threadIdx.y;
  int COL = blockIdx.x*blockDim.x+threadIdx.x;

  float tmpSum = 0;

  if (ROW < N && COL < N) {
      // each thread computes one element of the block sub-matrix
      for (int i = 0; i < N; i++) {
          tmpSum += A[ROW * N + i] * B[i * N + COL];
      }
  }
  C[ROW * N + COL] = tmpSum;
}

int main(int argc, char **argv) {
  int *A, *B, *C; // matrices

  // allocate memory for matrices
   A = (int *)malloc(N * N * sizeof(int));
   B = (int *)malloc(N * N * sizeof(int));
   C = (int *)malloc(N * N * sizeof(int));

  // initialize matrices with random values
    srand(0); // seed for reproducibility
    for (int i = 0; i < N * N; i++) {
      A[i] = rand() % 10;
      B[i] = rand() % 10;
    }
    printf("Matrix A:\n");
    print_matrix(A, N, N);
    printf("Matrix B:\n");
    print_matrix(B, N, N);
   const auto start_chrono = std::chrono::high_resolution_clock::now();

  // perform matrix multiplication on local matrices

  int THR_PER_BLOCK = 32;
  int THR_BLOCK_SIZE = N/THR_PER_BLOCK;

  dim3 blocksPerGrid(THR_BLOCK_SIZE, 1, 1);
  dim3 threadsPerBlock(THR_PER_BLOCK, 1, 1);

  matrixMultiplicationKernel<<<blocksPerGrid, threadsPerBlock>>>(A, B, C);


  const auto finish_chrono = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> diff = finish_chrono - start_chrono;

  //print the result matrix
  printf("Matrix C:\n");
  print_matrix(C, N, N);


  // free memory and finalize MPI
  free(A);
  free(B);
  free(C);

   std::cout << "Runtime: " << diff.count() << " s. " << std::endl;

  return 0;
}

