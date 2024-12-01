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

    //cout << endl << my_id << " ============================== "  << endl << endl;

    for(int i = my_id+1; i < 10000000; i = i + num_procs){
        
        if(isPrime(i)){
            cout << i << " | ";
        }

    }

    MPI_Finalize();
}

