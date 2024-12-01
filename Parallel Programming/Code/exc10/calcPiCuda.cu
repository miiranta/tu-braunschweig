#include <iostream>
#include <chrono>
#include <cmath>
#include <cuda_runtime.h>



#define PI 3.14159265358979323846  /* pi */

double integral(double boundaryLeft, double boundaryRight, long divisions);
double f(double x);

__global__ void integralCuda(double *res,double global_a, double global_b,long divisions, long number_of_processes){

    double tmp = 0.0;
    int th =threadIdx.x;
          //  long local_number_of_intervals = divisions / number_of_processes;
        double local_interval_width = (global_b - global_a) / (double)number_of_processes;

        double local_a = global_a + th * local_interval_width;
        double local_b = local_a + local_interval_width;

    double locIntervalWidth = (local_b - local_a) / (double)divisions;
    for (int i = 0; i < divisions; i++)
    {
        tmp += fdev(local_a + locIntervalWidth * ((double)i + 0.5)) * locIntervalWidth;
    }
    res[th]=tmp;

}

__device__ double fdev(double x)
{
    return (std::sqrt(1. - x * x));
}



int main(int argc, char **argv)
{
    int number_of_processes = 30;
    int my_rank = 0;
    const int repetitions = 1;

    double result = 0.0;
    double local_integral = 0.0;
    const auto start_chrono = std::chrono::high_resolution_clock::now();

    const long number_of_intervals = 5040 * 100000;

    const double global_a = 0.0;
    const double global_b = 1.0;
    double *res,*resHost;
    int numThreads=128;
    cudaMalloc(&res,numThreads*sizeof(double));
    resHost=(double*)malloc(numThreads*sizeof(double));
    for (int round = 0; round < repetitions; round++)
    {
    //     long local_number_of_intervals = number_of_intervals / number_of_processes;
    //     double local_interval_width = (global_b - global_a) / (double)number_of_processes;
    //    // #pragma omp parallel for default(shared) private(my_rank,local_integral)
    //     for (my_rank=0;my_rank<number_of_processes;my_rank++){

    //     double local_a = global_a + my_rank * local_interval_width;
    //     double local_b = local_a + local_interval_width;

    //     local_integral = integral(local_a, local_b, local_number_of_intervals);

        // send / receive
        
       //  result += local_integral;
       integralCuda<<<1,numThreads>>>(res,global_a,global_b,number_of_intervals,numThreads);
       cudaMemcpy(resHost,res,numThreads*sizeof(double), cudaMemcpyDeviceToHost);
        for (int i=0; i<numThreads;i++){result+=resHost[i];}

                 }
       // my_rank=0;

    }

    result = result / repetitions;

    const auto finish_chrono = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = finish_chrono - start_chrono;
    if (my_rank == 0)
    {
        std::cout << "Runtime: " << diff.count() << " s. " << std::endl;
        printf("Result: %0.20f \n", result * 4.);
        printf("Error: %0.20f \n", fabs(result * 4 - PI));
    }

    cudaFree(res);
    free(resHost);
    return 0;
}

double integral(double boundaryLeft, double boundaryRight, long divisions)
{
    double tmp = 0.0;
    double locIntervalWidth = (boundaryRight - boundaryLeft) / (double)divisions;
    //#pragma omp parallel for default(shared) private(tmp)
    for (int i = 0; i < divisions; i++)
    {
        tmp += f(boundaryLeft + locIntervalWidth * ((double)i + 0.5)) * locIntervalWidth;
    }
    return tmp;
}

double f(double x)
{
    return (std::sqrt(1. - x * x));
}
