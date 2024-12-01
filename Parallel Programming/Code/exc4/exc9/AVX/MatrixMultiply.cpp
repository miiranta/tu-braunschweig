#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <immintrin.h>

#define N 512 // matrix size


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
  for (int i=0; i<N;i++ ){
    for (int j=0; j<N; j++){
      C[i*N+j]=0;

      //  for (int k=0; k<N; k++){
      //    C[i*N+j]+=A[i*N+k]*B[k*N+j];
      //  }

      __m256i vecA, vecB, vecC;
      for(int k=0; k<N; k=k+8)
      {
        vecA = _mm256_loadu_si256((__m256i*)&A[i*N+k]);
        
        int locB[8];
        for(int ii=0; ii<0; ii++){locB[ii]=B[(k+ii)*N+j];}
        vecB = _mm256_loadu_si256((__m256i*)&B[0]);

        vecC = _mm256_mullo_epi32(vecA, vecB);
        int result[8];
        _mm256_storeu_si256((__m256i*)result, vecC);
        C[i*N+j] = result[0] + result[1] + result[2] + result[3] + result[4] + result[5] + result[6] + result[7]; 
      }

    }
  }
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