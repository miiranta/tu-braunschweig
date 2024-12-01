#include <iostream>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

using namespace std;

#define GRID_SIZE 8
#define ITERATIONS 1

struct Grid_tile {
    double value = 0;
    double new_value = 0;
    int iteration = 0;
};

struct Grid {
    int size = 0;

    Grid_tile* ghost_top;
    Grid_tile* ghost_bottom;
    Grid_tile* ghost_left; 
    Grid_tile* ghost_right;

    Grid_tile** main_grid;
};

MPI_Status status;
int my_id, num_procs;

void mainNode();
void gridNode();

Grid createGrid(int size);
Grid divideGrid(Grid grid, int proc_id);
void joinGrids(Grid &original, Grid grid, int proc_id);
void destroyGrid(Grid grid);
void printGrid(Grid grid);
void printGridNewValues(Grid grid);

void evaluateAll(Grid &grid);

void updateValues(Grid &grid);

void sendGrid(Grid &grid, int proc_id);
Grid recieveGrid(int proc_id);
void sendGhost(Grid &grid, int proc_id, int ghost_id);
void recieveGhost(Grid &grid, int proc_id);

//Main
int main(int argc, char** argv) 
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    //Conditions
    if(floor(sqrt(num_procs - 1)) != sqrt(num_procs - 1))
    {
        if(my_id == 0) {cout << "Number of nodes must be n**2 + 1" << endl;};
        MPI_Finalize();
        return 0;
    }
    if(num_procs == 1)
    {
        if(my_id == 0) {cout << "Number of nodes cannot be 1" << endl;};
        MPI_Finalize();
        return 0;
    }
    if((GRID_SIZE & (GRID_SIZE - 1)) != 0)
    {
        if(my_id == 0) {cout << "GRID_SIZE must be 2**n" << endl;};
        MPI_Finalize();
        return 0;
    }
    if(floor(GRID_SIZE/(sqrt(num_procs))) < 0)
    {
        if(my_id == 0) {cout << "GRID_SIZE too small." << endl;};
        MPI_Finalize();
        return 0;
    }

    //MAIN NODE
    if(my_id == 0) 
    {
       mainNode();
    }

    //GRID NODE
    else
    {
        gridNode();
    }

    MPI_Finalize();
}

//Nodes
void mainNode()
{

    //Create starting grid
    Grid starting = createGrid(GRID_SIZE);
    for(int i=0; i<starting.size; i++)
    {
        for(int j=0; j<starting.size; j++)
        {
            starting.main_grid[i][j].value = 0;
        }
    }
    for(int i=0; i<starting.size; i++) starting.ghost_left[i].value = 50;
    for(int i=0; i<starting.size; i++) starting.ghost_top[i].value = 150;

    //Distribute
    for(int i=1; i<num_procs; i++)
    {
        Grid sen = divideGrid(starting, i);
        sendGrid(sen, i);
    }

    //Receive
    Grid result = createGrid(GRID_SIZE);
    for(int i=1; i<num_procs; i++)
    {
        Grid grid = recieveGrid(i);
        joinGrids(result, grid, i);
    }

    //Print result
    printGrid(result);

    return;
}

void gridNode()
{
    //Receive
    Grid grid = recieveGrid(0);

    //Get x,y based on id
    int proc_id = my_id - 1;
    int x = 0;
    int y = 0;
    int rotate = GRID_SIZE / grid.size;
    x = proc_id % rotate;
    y = floor(proc_id / rotate);

    //Loop
    for(int b = 0; b < ITERATIONS; b++)
    {
        cout << "OOOOOOOOOO " << my_id << endl;

        //Calculate
        evaluateAll(grid);
        updateValues(grid);

        //SEN ghosts
        if(y == 0)
        {
            //Do nothing
        }
        else
        {
            cout << my_id << ": Sending top to: " << (x + (y-1)*rotate) << endl;
            sendGhost(grid, (x + (y-1)*rotate), 0);
            cout << my_id << ": Receiving bottom from: " << (x + (y-1)*rotate) << endl;
            recieveGhost(grid, (x + (y-1)*rotate));
        }

        if(y == (floor(sqrt(num_procs - 1))-1)) //Copy from ghost_bottom
        {
            //Do nothing
        }
        else 
        {
            cout << my_id << ": Sending bottom to: " << (x + (y+1)*rotate) << endl;
            sendGhost(grid, (x + (y+1)*rotate), 1);
            cout << my_id << ": Receiving top from: " << (x + (y+1)*rotate) << endl;
            recieveGhost(grid, (x + (y+1)*rotate));
        }

        if(x == 0)
        {
            //Do nothing
        }
        else
        {
            cout << my_id << ": Sending left to: " << (x-1) + (y)*rotate << endl;
            sendGhost(grid, (x-1) + (y)*rotate, 2);
            cout << my_id << ": Receiving right from: " << (x-1) + (y)*rotate << endl;
            recieveGhost(grid, (x-1) + (y)*rotate);
        }

        if(x == (floor(sqrt(num_procs - 1))-1)) //Copy from ghost_right
        {
            //Do nothing
        }
        else
        {
            cout << my_id << ": Sending right to: " << (x+1) + (y)*rotate << endl;
            sendGhost(grid, (x+1) + (y)*rotate, 3);
            cout << my_id << ": Receiving left from: " << (x+1) + (y)*rotate << endl;
            recieveGhost(grid, (x+1) + (y)*rotate);
        }


    }
    
    //Send back
    sendGrid(grid, 0);

    return;
}

//Grid handling
Grid createGrid(int size)
{
    Grid grid = Grid();
    grid.size = size;

    //ALLOC
    grid.ghost_top = new Grid_tile[size];
    grid.ghost_bottom = new Grid_tile[size];
    grid.ghost_left = new Grid_tile[size];
    grid.ghost_right = new Grid_tile[size];

    grid.main_grid = new Grid_tile*[size];
    for(int i=0; i<size; i++)
    {
        grid.main_grid[i] = new Grid_tile[size];
    }

    //POP
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            grid.main_grid[i][j].value = 0;
        }
    }

    for(int i=0; i<size; i++)
    {
        grid.ghost_top[i].value = 0;
    }

    for(int i=0; i<size; i++)
    {
        grid.ghost_bottom[i].value = 0;
    }

    for(int i=0; i<size; i++)
    {
        grid.ghost_left[i].value = 0;
    }

    for(int i=0; i<size; i++)
    {
        grid.ghost_right[i].value = 0;
    }

    return grid;
}

Grid divideGrid(Grid grid, int proc_id)
{
    if((proc_id < 1) || (proc_id > (num_procs-1)))
    {   
        cout << "Invalid proc_id" << endl;
        return Grid();
    }

    int new_size = grid.size / floor(sqrt(num_procs - 1));
    Grid new_grid = createGrid(new_size);

    //Get x,y based on id
    proc_id = proc_id - 1;
    int x = 0;
    int y = 0;
    int rotate = grid.size / new_size;
    x = proc_id % rotate;
    y = floor(proc_id / rotate);

    //COPY main from big grid
    for(int i=0; i<new_size; i++)
    {
        for(int j=0; j<new_size; j++)
        {
            new_grid.main_grid[i][j].value = grid.main_grid[i + new_size*x][j + new_size*y].value;
        }
    }

    //COPY ghost_top
    if(y == 0) //Copy from ghost_top
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_top[i].value =  grid.ghost_top[i + new_size*x].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_top[i].value = grid.main_grid[i + new_size*x][new_size*y - 1].value;
        }
    }

    //COPY ghost_bottom
    if(y == (floor(sqrt(num_procs - 1))-1)) //Copy from ghost_bottom
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_bottom[i].value = grid.ghost_bottom[i + new_size*x].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_bottom[i].value = grid.main_grid[i + new_size*x][new_size*(y+1)].value;
        }
    }

    //COPY ghost_left
    if(x == 0) //Copy from ghost_left
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_left[i].value =  grid.ghost_left[i + new_size*y].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_left[i].value = grid.main_grid[new_size*x - 1][i + new_size*y].value;
        }
    }

    //COPY ghost_right
    if(x == (floor(sqrt(num_procs - 1))-1)) //Copy from ghost_right
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_right[i].value =  grid.ghost_right[i + new_size*y].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<new_size; i++)
        {
            new_grid.ghost_right[i].value = grid.main_grid[new_size*(x+1)][i + new_size*y].value;
        }
    }

    return new_grid;
}

void joinGrids(Grid &original, Grid grid, int proc_id)
{
    if((proc_id < 1) || (proc_id > (num_procs-1)))
    {   
        cout << "Invalid proc_id" << endl;
        return;
    }

    //Get x,y based on id
    proc_id = proc_id - 1;
    int x = 0;
    int y = 0;
    int rotate = original.size / grid.size;
    x = proc_id % rotate;
    y = floor(proc_id / rotate);

    //COPY main from big grid
    for(int i=0; i<grid.size; i++)
    {
        for(int j=0; j<grid.size; j++)
        {
            original.main_grid[i + grid.size*x][j + grid.size*y].value = grid.main_grid[i][j].value;
        }
    }

    //COPY ghost_top
    if(y == 0) //Copy from ghost_top
    {
        for(int i=0; i<grid.size; i++)
        {
            original.ghost_top[i + grid.size*x].value = grid.ghost_top[i].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<grid.size; i++)
        {
            original.main_grid[i + grid.size*x][grid.size*y - 1].value = grid.ghost_top[i].value;
        }
    }

    //COPY ghost_bottom
    if(y == (floor(sqrt(num_procs - 1))-1)) //Copy from ghost_bottom
    {
        for(int i=0; i<grid.size; i++)
        {
            original.ghost_bottom[i + grid.size*x].value = grid.ghost_bottom[i].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<grid.size; i++)
        {
            original.main_grid[i + grid.size*x][grid.size*(y+1)].value = grid.ghost_bottom[i].value;
        }
    }

    //COPY ghost_left
    if(x == 0) //Copy from ghost_left
    {
        for(int i=0; i<grid.size; i++)
        {
            original.ghost_left[i + grid.size*y].value =  grid.ghost_left[i].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<grid.size; i++)
        {
            original.main_grid[grid.size*x - 1][i + grid.size*y].value = grid.ghost_left[i].value;
        }
    }

    //COPY ghost_right
    if(x == (floor(sqrt(num_procs - 1))-1)) //Copy from ghost_right
    {
        for(int i=0; i<grid.size; i++)
        {
            original.ghost_right[i + grid.size*y].value =  grid.ghost_right[i].value;
        }
    }
    else //Copy from main
    {
        for(int i=0; i<grid.size; i++)
        {
            original.main_grid[grid.size*(x+1)][i + grid.size*y].value = grid.ghost_right[i].value;
        }
    }

    return;
}

void destroyGrid(Grid grid)
{
    for(int i=0; i<grid.size; i++)
    {
        delete grid.main_grid[i];
    }
    delete grid.main_grid;

    delete grid.ghost_bottom;
    delete grid.ghost_left;
    delete grid.ghost_right;
    delete grid.ghost_top;
}

void printGrid(Grid grid)
{
    int size = grid.size;

    cout << "Central:  " << endl;
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            cout << grid.main_grid[i][j].value << "  ";
        }
        cout << endl;
    }

    cout << "Top:  " << endl;
    for(int i=0; i<size; i++)
    {
        cout << grid.ghost_top[i].value << "  ";
    }
    cout << endl;

    cout << "Bottom:  " << endl;
    for(int i=0; i<size; i++)
    {
        cout << grid.ghost_bottom[i].value << "  ";
    }
    cout << endl;

    cout << "Left:  " << endl;
    for(int i=0; i<size; i++)
    {
        cout << grid.ghost_left[i].value << "  ";
    }
    cout << endl;

    cout << "Right:  " << endl;
    for(int i=0; i<size; i++)
    {
        cout << grid.ghost_right[i].value << "  ";
    }
    cout << endl;
}

void printGridNewValues(Grid grid)
{
    int size = grid.size;

    cout << "New values:  " << endl;
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            cout << grid.main_grid[i][j].new_value << "  ";
        }
        cout << endl;
    }
}

//Evaluate
void evaluateAll(Grid &grid)
{
    for(int i=0; i<grid.size; i++)
    {
        for(int j=0; j<grid.size; j++)
        {
            double sum = grid.main_grid[i][j].value;

            //Average of 4 neighbors
            if(i > 0) sum += grid.main_grid[i-1][j].value;	//Top
            else sum += grid.ghost_top[j].value;

            if(i < grid.size-1) sum += grid.main_grid[i+1][j].value;	//Bottom
            else sum += grid.ghost_bottom[j].value;

            if(j > 0) sum += grid.main_grid[i][j-1].value;	//Left
            else sum += grid.ghost_left[i].value;

            if(j < grid.size-1) sum += grid.main_grid[i][j+1].value;	//Right
            else sum += grid.ghost_right[i].value;

            grid.main_grid[i][j].new_value = sum / 5; 

        }
    }

}

//Send and Recieve
void sendGrid(Grid &grid, int proc_id)
{
    //Size
    int sen1 = grid.size;
    MPI_Ssend(&sen1, 1, MPI_INT, proc_id, 0, MPI_COMM_WORLD);

    //Ghost top
    double sen2[sen1];
    for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_top[i].value;}
    MPI_Ssend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD);

    //Ghost bottom
    for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_bottom[i].value;}
    MPI_Ssend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD);

    //Ghost left
    for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_left[i].value;}
    MPI_Ssend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD);

    //Ghost right
    for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_right[i].value;}
    MPI_Ssend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD);

    //Main
    for(int j=0; j<sen1; j++)
    {
        for(int i=0; i<sen1; i++){sen2[i] = grid.main_grid[j][i].value;}
        MPI_Ssend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD);
    }

}

Grid recieveGrid(int proc_id)
{
    //Size
    int rec1;
    MPI_Recv(&rec1, 1, MPI_INT, proc_id, 0, MPI_COMM_WORLD, &status); 
    Grid grid = createGrid(rec1);

    //Ghost top
    double rec2[rec1];
    MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
    for(int i=0; i<rec1; i++){grid.ghost_top[i].value = rec2[i];}

    //Ghost bottom
    MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
    for(int i=0; i<rec1; i++){grid.ghost_bottom[i].value = rec2[i];}

    //Ghost left
    MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
    for(int i=0; i<rec1; i++){grid.ghost_left[i].value = rec2[i];}

    //Ghost right
    MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
    for(int i=0; i<rec1; i++){grid.ghost_right[i].value = rec2[i];}

    //Main
    for(int j=0; j<rec1; j++)
    {
        MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
        for(int i=0; i<rec1; i++){grid.main_grid[j][i].value = rec2[i];}
    }

    return grid;
}

void sendGhost(Grid &grid, int proc_id, int ghost_id)
{
    MPI_Request req;

    //Size
    int sen1 = grid.size;
    MPI_Isend(&sen1, 1, MPI_INT, proc_id, 0, MPI_COMM_WORLD, &req);

    //Ghost id
    MPI_Isend(&ghost_id, 1, MPI_INT, proc_id, 0, MPI_COMM_WORLD, &req);

    double sen2[sen1];
    if(ghost_id == 0) //TOP
    {
        for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_top[i].value;}
        MPI_Isend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &req);
    }

    else if(ghost_id == 1) //BOTTOM
    {
        for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_bottom[i].value;}
        MPI_Isend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &req);
    }

    else if(ghost_id == 2) //LEFT
    {
        for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_left[i].value;}
        MPI_Isend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &req);
    }

    else if(ghost_id == 3) //RIGHT
    {
        for(int i=0; i<sen1; i++){sen2[i] = grid.ghost_right[i].value;}
        MPI_Isend(sen2, sen1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &req);
    }

}

void recieveGhost(Grid &grid, int proc_id)
{
    //Size
    int rec1;
    MPI_Recv(&rec1, 1, MPI_INT, proc_id, 0, MPI_COMM_WORLD, &status); 
    
    //Test sizes
    if(grid.size != rec1)
    {
        cout << "Incompatible grid sizes" << endl;
    }

    //Ghost id
    int ghost_id;
    MPI_Recv(&ghost_id, 1, MPI_INT, proc_id, 0, MPI_COMM_WORLD, &status); 

    double rec2[rec1];
    if(ghost_id == 0) //TOP
    {
        MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
        for(int i=0; i<rec1; i++){grid.ghost_bottom[i].value = rec2[i];}
    }

    else if(ghost_id == 1) //BOTTOM
    {
        MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
        for(int i=0; i<rec1; i++){grid.ghost_top[i].value = rec2[i];}
    }

    else if(ghost_id == 2) //LEFT
    {
        MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
        for(int i=0; i<rec1; i++){grid.ghost_right[i].value = rec2[i];}
    }

    else if(ghost_id == 3) //RIGHT
    {
        MPI_Recv(&rec2, rec1, MPI_DOUBLE, proc_id, 0, MPI_COMM_WORLD, &status); 
        for(int i=0; i<rec1; i++){grid.ghost_left[i].value = rec2[i];}
    }

}

//Etc
void updateValues(Grid &grid)
{
    for(int i=0; i<grid.size; i++)
    {
        for(int j=0; j<grid.size; j++)
        {
            grid.main_grid[i][j].value = grid.main_grid[i][j].new_value;
        }
    }
}