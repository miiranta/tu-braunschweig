#include <iostream>
#include <chrono>
#include <cmath>

#include <omp.h>

#define PI 3.14159265358979323846  /* pi */

double integral(double boundaryLeft, double boundaryRight, long divisions);
double f(double x);



int main(int argc, char **argv)
{
    int number_of_processes = 0;
    int my_rank = 0;
    double result = 0.0;
    double* result_g = new double[number_of_processes];

    #pragma omp parallel private(my_rank, result)

    number_of_processes = omp_get_num_threads();
    my_rank = omp_get_thread_num();
    const int repetitions = 1;

    
    
    double local_integral = 0.0;
    const auto start_chrono = std::chrono::high_resolution_clock::now();

    const long number_of_intervals = 5040 * 100000;

    const double global_a = 0.0;
    const double global_b = 1.0;

    std::cout << my_rank << " of " << number_of_processes << std::endl;

    
    for (int round = 0; round < repetitions; round++)
    {

        const long local_number_of_intervals = number_of_intervals / number_of_processes;
        const double local_interval_width = (global_b - global_a) / (double)number_of_processes;
        const double local_a = global_a + my_rank * local_interval_width;
        const double local_b = local_a + local_interval_width;

        local_integral = integral(local_a, local_b, local_number_of_intervals);

        result += local_integral;

    }

    result = result / repetitions;
    result_g[my_rank] = result;

    const auto finish_chrono = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = finish_chrono - start_chrono;
    if (my_rank == 0)
    {
        result = 0.0;
        for(int i = 0; i < number_of_processes; i++)
        {
            result += result_g[i];
        }

        std::cout << "Runtime: " << diff.count() << " s. " << std::endl;
        printf("Result: %0.20f \n", result * 4.);
        printf("Error: %0.20f \n", fabs(result * 4 - PI));
    }

    return 0;
}

double integral(double boundaryLeft, double boundaryRight, long divisions)
{
    double tmp = 0.0;
    double locIntervalWidth = (boundaryRight - boundaryLeft) / (double)divisions;
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
