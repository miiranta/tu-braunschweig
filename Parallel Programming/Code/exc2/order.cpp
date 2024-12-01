#include <iostream>
#include <stdio.h>
#include <mpi.h>

using namespace std;


int main(int argc, char** argv) {

    MPI_Status status;
    int my_id, num_procs;
    int tag=0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    //cout << num_procs << " - " << my_id << endl;

    


    if(my_id == 0){
        double rec;

        for(int i = 1; i < num_procs; i++){
            MPI_Recv(&rec, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status); 
            cout << rec << endl;
        }
        
    }

    else{
        double sen = my_id;
        int dest = 0;

        MPI_Ssend(&sen, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
         
    }


    MPI_Finalize();
}
