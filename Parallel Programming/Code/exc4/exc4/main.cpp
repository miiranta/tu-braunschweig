#include <iostream>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

using namespace std;

double f(double x)
{
    return (std::sqrt(1. - x * x));
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

int main(int argc, char** argv) {

    MPI_Status status;
    int my_id, num_procs;

    const int repetitions = 1;

    double result = 0.0;
    double local_integral = 0.0;

    const long number_of_intervals = 5040 * 100000;

    const double global_a = 0.0;
    const double global_b = 1.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    //Main
    if(my_id == 0) {

        double amount = 0;
        for(int i = 1; i < num_procs; i++){
            double rec;
            MPI_Recv(&rec, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status); 
            cout << rec << endl;
            amount = amount + rec;
        }

        cout << "SUM: " << amount << endl;
        cout << "SUM * 4: " << amount*4 << endl;
    }

    //Node
    else
    {
        
        num_procs = num_procs - 1;
        my_id = my_id - 1;
        for (int round = 0; round < repetitions; round++)
            {

                const long local_number_of_intervals = number_of_intervals / num_procs;
                const double local_interval_width = (global_b - global_a) / (double)num_procs;
                const double local_a = global_a + my_id * local_interval_width;
                const double local_b = local_a + local_interval_width;

                local_integral = integral(local_a, local_b, local_number_of_intervals);

                // send / receive
                
                result += local_integral;

            }

            result = result / repetitions;


        double sen = result;
        int dest = 0;
        MPI_Ssend(&sen, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}

