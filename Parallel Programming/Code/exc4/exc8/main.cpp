#include <iostream>
#include <chrono>
#include <cmath>

#include <thread>

#define PI 3.14159265358979323846  /* pi */

double integral(double boundaryLeft, double boundaryRight, long divisions);
double f(double x);
int happiness(int i, int n);

double* result_g;

int main(int argc, char **argv)
{
   
    int nro_t = 1;
    const auto start_chrono = std::chrono::high_resolution_clock::now();

    result_g = new double[nro_t];

    std::thread t[nro_t];
    for(int i = 0; i < nro_t; i++)
    {
        t[i] = std::thread(happiness, i, nro_t);
    }

    for(int i = 0; i < nro_t; i++)
    {
        t[i].join();
    }

    double result = 0.0;
    for(int i = 0; i < nro_t; i++)
    {
        result += result_g[i];
    }

    const auto finish_chrono = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = finish_chrono - start_chrono;

    std::cout << "Runtime: " << diff.count() << " s. " << std::endl;
    printf("Result: %0.20f \n", result * 4.);
    printf("Error: %0.20f \n", fabs(result * 4 - PI));
    
    return 0;
}

int happiness(int i, int n)
{
    int number_of_processes = n;
    int my_rank = i;
    const int repetitions = 1;

    double result = 0.0;
    double local_integral = 0.0;

    const long number_of_intervals = 5040 * 100000;

    const double global_a = 0.0;
    const double global_b = 1.0;

    for (int round = 0; round < repetitions; round++)
    {

        const long local_number_of_intervals = number_of_intervals / number_of_processes;
        const double local_interval_width = (global_b - global_a) / (double)number_of_processes;
        const double local_a = global_a + my_rank * local_interval_width;
        const double local_b = local_a + local_interval_width;

        local_integral = integral(local_a, local_b, local_number_of_intervals);

        // send / receive
        result += local_integral;

    }

    result = result / repetitions;

    result_g[i] = result;

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
