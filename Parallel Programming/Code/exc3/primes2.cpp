#include <iostream>
#include <stdio.h>
#include <mpi.h>

using namespace std;

bool isPrime(int x) {
    for (int d = 2; d * d <= x; d++) {
        if (x % d == 0)
            return false;
    }
    return x >= 2;
}

int main(int argc, char** argv) {

    MPI_Status status;
    int my_id, num_procs;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    if(my_id == 0) {

        double amount = 0;
        for(int i = 1; i < num_procs; i++){
            double rec;
            MPI_Recv(&rec, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status); 
            cout << rec << endl;
            amount = amount + rec;
        }

        printf("%f\n", amount);
    }

    else
    {
        double amount = 0;
        for(int i = my_id; i < 10000000; i = i + (num_procs-1))
        {
            
            if(isPrime(i)){
                amount++;
            }

        }
        double sen = amount;
        int dest = 0;
        MPI_Ssend(&sen, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
    }

    

    MPI_Finalize();
}

