#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
//#include <immintrin.h>
#include <cuda_runtime.h>

#define N 32 // matrix size


// function to print a matrix
void print_matrix(int *matrix, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      printf("%d ", matrix[i * cols + j]);
    }
    printf("\n");
  }
}


__global__ void calcC(int * A, int * B, int *C,int NN){
int i=blockIdx.x*blockDim.x+threadIdx.x;
int j=blockIdx.y*blockDim.y+threadIdx.y;
C[i*NN+j]=0;
for (int k=0; k<NN;k++){
C[i*NN+j]+=A[i*NN+k]*B[k*NN+j];
}
}

int main(int argc, char **argv) {
  int *A, *B, *C; // matrices

  // allocate memory for matrices
   A = (int *)malloc(N * N * sizeof(int));
   B = (int *)malloc(N * N * sizeof(int));
   C = (int *)malloc(N * N * sizeof(int));
int *Adev,*Bdev,*Cdev;

cudaMalloc(&Adev,N*N*sizeof(int));
cudaMalloc(&Bdev,N*N*sizeof(int));
cudaMalloc(&Cdev,N*N*sizeof(int));


  // initialize matrices with random values
    srand(0); // seed for reproducibility
    for (int i = 0; i < N * N; i++) {
      A[i] = rand() % 10;
      B[i] = 0;//rand() % 10;
    }
for (int i=0;i<N;i++){B[i*N+i]=1;}
    printf("Matrix A:\n");
    print_matrix(A, N, N);
    printf("Matrix B:\n");
    print_matrix(B, N, N);
   const auto start_chrono = std::chrono::high_resolution_clock::now();
cudaMemcpy(Adev,A,N*N*sizeof(int),cudaMemcpyHostToDevice);
cudaMemcpy(Bdev,B,N*N*sizeof(int),cudaMemcpyHostToDevice);
  // perform matrix multiplication on local matrices
  //for (int i=0; i<N;i++ ){
    //for (int j=0; j<N; j++){
      //C[i*N+j]=0;

    //   for (int k=0; k<N; k++){
    //     C[i*N+j]+=A[i*N+k]*B[k*N+j];
    //   }

    //}
  //}
dim3 numBlocks(N,N,1);
dim3 numThreads(1,1,1);

calcC<<<numBlocks,numThreads>>>(Adev,Bdev,Cdev,N);
cudaMemcpy(C,Cdev,N*N*sizeof(int),cudaMemcpyDeviceToHost);
    const auto finish_chrono = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = finish_chrono - start_chrono;

   //print the result matrix
    printf("Matrix C:\n");
    print_matrix(C, N, N);


  // free memory and finalize MPI
  free(A);
  free(B);
  free(C);

cudaFree(Adev);
cudaFree(Bdev);
cudaFree(Cdev);

   std::cout << "Runtime: " << diff.count() << " s. " << std::endl;

  return 0;
}
