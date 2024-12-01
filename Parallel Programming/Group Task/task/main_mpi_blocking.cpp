#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mpi.h>
#include <time.h>
#include <cmath>
#include <math.h>

#include "Mapper2D.h"
#include "Parameter.h"
#include "utilities.h"

int main()
{
    // VARS -------------------------------------------------------------------------

    // Initialize timer 
    Timer timer;

    // Initialize iteration variables
    int iteration = 0;
    bool done = false;
    bool alldone = false;

    // Initialize simulation parameters
    constexpr SimulationParameter parameter;

    // Intitialize MPI variables
    int size, myRank;
    MPI_Status status;

    // Intitialize MPI
    MPI_Init(NULL,NULL); 
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Verify size constraint, must be a n.n grid
    if(floor(sqrt(size)) != sqrt(size)) 
    {
        if(myRank == 0) printf("Number of processors must be nxn!\n");
        MPI_Finalize();
        return 0;
    }

    // Intitialize the grid size
    const int xy = sqrt(size);
    const int numPartsX = xy;
    const int numPartsY = xy;

    const int localNx = parameter.gridNx / numPartsX; 
    const int localNy = parameter.gridNy / numPartsY;

    const int realGridNx = localNx * numPartsX;
    const int realGridNy = localNy * numPartsY;

    // Initialize (grid) map
    const Mapper2D innerGrid(localNx, localNy);

    // Initialize (grid + ghost Layers) map
    const Mapper2D entireGrid(innerGrid.nx() + 2, innerGrid.ny() + 2);

    // Initialize topology
    Mapper2D processTopology = Mapper2D(numPartsX, numPartsY);
    int myXRank = processTopology.xForPos(myRank);
    int myYRank = processTopology.yForPos(myRank);

    // Initialize 2 grids
    // Now we can apply the calculations more easily
    std::vector<double> oldData (entireGrid.size());
    std::vector<double> newData (entireGrid.size());

    // Initialize MPI receive buffers
    double *leftReceiveBuffer   = new double[innerGrid.ny()];
    double *rightReceiveBuffer  = new double[innerGrid.ny()];
    double *topReceiveBuffer    = new double[innerGrid.nx()];
    double *bottomReceiveBuffer = new double[innerGrid.nx()];

    // Initialize MPI send buffers
    double *leftSendBuffer      = new double[innerGrid.ny()];
    double *rightSendBuffer     = new double[innerGrid.ny()]; 
    double *topSendBuffer       = new double[innerGrid.nx()]; 
    double *bottomSendBuffer    = new double[innerGrid.nx()];  

    // ------------------------------------------------------------------------------

    // Set all values to 0
    // Avoiding trash that could be on the memory
    for (size_t i = 0; i < entireGrid.size(); i++)
    {
        oldData[i] = 0.0;
    } 
   
    // Set values of the ghost layers (in this case, the receive buffers) with the parameters we want to simulate
    // It depends if this piece of the grid is one of the 'borders' of the whole grid
    if (myXRank == 0)
        for (size_t i = 0; i < innerGrid.ny(); i++)
            leftReceiveBuffer[i] = parameter.bcLeft;

    else if (myXRank == (numPartsX - 1))
        for (size_t i = 0; i < innerGrid.ny(); i++)
            rightReceiveBuffer[i] = parameter.bcRight;

    if (myYRank == 0)
        for (size_t i = 0; i < innerGrid.nx(); i++)
            topReceiveBuffer[i] = parameter.bcTop;

    else if (myYRank == (numPartsY-1))
        for (size_t i = 0; i < innerGrid.nx(); i++)
            bottomReceiveBuffer[i] = parameter.bcBottom;
    
    // Start timer
    timer.startNupsTimer();

    // Iterate
    while (!alldone)
    {
        // Initialize error variables
        // They will change every iteration, so it's generally safer to keep it local
        double error = 0.0;
        double diff;

        // Exchange ---
        // the ghost layers with neighbours
        // In this case, wait for all the responses to start calculations

        // 1
        // Even exchanges with right - Odd exchanges with left
        if ((myXRank % 2) == 0 && myXRank != (numPartsX-1))
        { 
            // If even send right 
            // Can not be the furthest to the right
            MPI_Sendrecv(rightSendBuffer,innerGrid.ny(), MPI_DOUBLE, (myRank+1),0,rightReceiveBuffer, innerGrid.ny() ,MPI_DOUBLE, (myRank+1), 0, MPI_COMM_WORLD, &status);
        }
        else if ((myXRank % 2) != 0)
        { 
            // If odd get from left 
            MPI_Sendrecv(leftSendBuffer, innerGrid.ny(), MPI_DOUBLE, (myRank-1), 0,leftReceiveBuffer,innerGrid.ny(),MPI_DOUBLE,(myRank-1),0,MPI_COMM_WORLD,&status);
        }

        // 1
        // Even exchanges with left - Odd exchanges with right
        if (myXRank % 2 == 0 && myXRank != 0)
        {
            // If even send left
            // Can not be the furthest to the left
            MPI_Sendrecv(leftSendBuffer, innerGrid.ny(), MPI_DOUBLE, (myRank-1), 0,leftReceiveBuffer,innerGrid.ny(),MPI_DOUBLE,(myRank-1),0,MPI_COMM_WORLD,&status);
        }
        else if (myXRank % 2 != 0 && myXRank != (numPartsX-1))
        {
            // If odd get from right
            MPI_Sendrecv(rightSendBuffer,innerGrid.ny(), MPI_DOUBLE, (myRank+1),0,rightReceiveBuffer, innerGrid.ny() ,MPI_DOUBLE, (myRank+1), 0, MPI_COMM_WORLD, &status);
        }

        // 2
        // Even exchanges with bottom - Odd exchanges with top
        if (myYRank % 2 == 0 && myYRank != (numPartsY-1))
        { 
            // If even send bottom
            // Can not be the furthest to the bottom
            MPI_Sendrecv(bottomSendBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank+numPartsX), 0,bottomReceiveBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank+numPartsX), 0, MPI_COMM_WORLD, &status); 
        }
        else if (myYRank % 2 != 0)
        { 
            // If odd get from top
            MPI_Sendrecv(topSendBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank-numPartsX), 0, topReceiveBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank-numPartsX), 0, MPI_COMM_WORLD, &status);
        }

        // 2
        // Even exchanges with top - Odd exchanges with bottom
        if (myYRank % 2 == 0 && myYRank != 0)
        {
            // If even send top
            // Can not be the furthest to the top
            MPI_Sendrecv(topSendBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank-numPartsX), 0, topReceiveBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank-numPartsX), 0, MPI_COMM_WORLD, &status);       
        }
        else if (myYRank % 2 != 0 && myYRank != (numPartsY-1))
        {
            // If odd get from bottom
            MPI_Sendrecv(bottomSendBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank+numPartsX), 0, bottomReceiveBuffer, innerGrid.nx(), MPI_DOUBLE, (myRank+numPartsX), 0, MPI_COMM_WORLD, &status);    
        }
        
        // ---

        // Copy from buffer to arrays ---

        // Copy received buffer to ghost layer (TOP and BOTTOM)
        for (size_t x = 1; x < entireGrid.nx() - 1; x++)
        {
            oldData[entireGrid.pos(x, 0)] = topReceiveBuffer[x - 1]; // left part is ghost layer right is recieve buffer
            oldData[entireGrid.pos(x, entireGrid.ny() - 1)] = bottomReceiveBuffer[x - 1];
        }

        // Copy received buffer to ghost layer (LEFT and RIGHT)
        for (size_t y = 1; y < entireGrid.ny() - 1; y++)
        {
            oldData[entireGrid.pos(0, y)] = leftReceiveBuffer[y - 1];
            oldData[entireGrid.pos(entireGrid.nx() - 1, y)] = rightReceiveBuffer[y - 1];
        }

        // ---

        // Evaluate ---
 
        // For every slot, evaluate
        for (size_t y = 1; y < entireGrid.ny() - 1; y++)
        {
            for (size_t x = 1; x < entireGrid.nx() - 1; x++)
            {
                
                newData[entireGrid.pos(x, y)] = 0.25 * (oldData[entireGrid.pos(x - 1, y)] +
                                                        oldData[entireGrid.pos(x + 1, y)] +
                                                        oldData[entireGrid.pos(x, y - 1)] +
                                                        oldData[entireGrid.pos(x, y + 1)]);
                
                diff = newData[entireGrid.pos(x, y)] - oldData[entireGrid.pos(x, y)];
                error = error + diff * diff;
            }
        }

        // ---

        // Copy from arrays to buffers  ---
        
        // Top and bottom
        for (size_t x = 1; x < entireGrid.nx() - 1; x++) // Whole grid
        {
            topSendBuffer[x-1] = newData[entireGrid.pos(x,1)];
            bottomSendBuffer[x-1] = newData[entireGrid.pos(x,entireGrid.ny()-2)];
        }

        // Left and right
        for (size_t y = 2; y < entireGrid.ny() - 2; y++)
        {
            leftSendBuffer[y-1] = newData[entireGrid.pos(1,y)];
            rightSendBuffer[y-1] = newData[entireGrid.pos(entireGrid.nx()-2,y)];
        }

        // Corners
        leftSendBuffer[0] = newData[entireGrid.pos(1,1)];
        leftSendBuffer[entireGrid.ny()-3] = newData[entireGrid.pos(1,entireGrid.ny()-2)];
        rightSendBuffer[0] = newData[entireGrid.pos(entireGrid.nx()-2,1)];
        rightSendBuffer[entireGrid.ny()-3] = newData[entireGrid.pos(entireGrid.nx()-2, entireGrid.ny()-2)];

        // ---

        // Copy new data to old data 
        std::swap(oldData, newData);

        // Every 'outputInterval', print
        iteration++;
        if ((iteration % parameter.outputInterval) == 0)
        {
            // Print
            const auto mnups = timer.getMNups(innerGrid.size() * parameter.outputInterval);
            std::cout << "time step: " << iteration << " error: " << error << " MNUPS: " << mnups << " Rank: " << myRank << "\n";
            timer.startNupsTimer();
        }

        // Stopping criteria
        done = (error < parameter.maxError) || (iteration >= parameter.maxIterations);

        MPI_Allreduce(&done,&alldone,1,MPI_C_BOOL,MPI_LAND,MPI_COMM_WORLD);

    }

    // Process 0
    // Collect data and save result
    if (myRank == 0)
    {
        // Initialize a result grid
        // main grid
        Mapper2D globalGrid = Mapper2D(realGridNx, realGridNy);
        double *resultData = new double[realGridNx * realGridNy];

        // Initialize a result grid
        // main grid + ghost layers
        double *wholeData = new double[(realGridNx+2) * (realGridNy+2)];
        Mapper2D wholeGrid = Mapper2D(realGridNx+2, realGridNy+2);
        
        // Copy data 
        // from process 0 grid > result grid
        for (size_t x = 1; x < entireGrid.nx() - 1; x++)
            for (size_t y = 1; y < entireGrid.ny() - 1; y++)
                resultData[globalGrid.pos(x - 1, y - 1)] = oldData[entireGrid.pos(x, y)];

        // Copy data 
        // from NON process 0 grids > result grid
        for (int partX = 0; partX < numPartsX; partX++)
            for (int partY = 0; partY < numPartsY; partY++)
                if (partX || partY) //if (!(i==0 && j==0)
                {
                    std::cout << "Partition X = " << partX << ", Partition Y = " << partY << std::endl;
                    for (size_t y = 0; y < entireGrid.ny() - 2; y++){ // line by line
                        MPI_Recv(resultData + globalGrid.pos(partX * localNx, partY * localNy + y), entireGrid.nx() - 2, MPI_DOUBLE, processTopology.pos(partX, partY), 0, MPI_COMM_WORLD, &status);
                    }
                }
        
        // Copy data 
        // ghost layers + results > whole grid
        int c = 0;
        for (size_t i = 0; i < wholeGrid.size(); i++){
            if (i < wholeGrid.nx())
            {
                wholeData[i] = parameter.bcTop;
            }
            else if (i > (wholeGrid.size()-wholeGrid.nx()-1))
            {
                wholeData[i] = parameter.bcBottom;
            }
            else if (i % wholeGrid.nx() == 0)
            {
                wholeData[i] = parameter.bcLeft;
            }
            else if (wholeGrid.xForPos(i) % (wholeGrid.nx()-1) == 0)
            {
                wholeData[i] = parameter.bcRight;
            }
            else
            {
                wholeData[i] = resultData[c];
                c++;
            }
        }
       
        // Save paraview file
        writeUCDFile("inp/inp_BlockingOut.inp", wholeData, wholeGrid);
        
        // Print result stats
        const auto runtime = timer.getRuntimeSeconds();
        std::cout << "Runtime: " << runtime << " s. " << std::endl;
        std::cout << "Average MNUPS: " << timer.getAverageMNups(innerGrid.size() * iteration) << std::endl;
        
        delete[] resultData;
    }

    // Process not 0
    // Send data to process 0
    else
    {
        for (size_t y = 1; y < entireGrid.ny() - 1; y++)
            MPI_Ssend(oldData.data() + entireGrid.pos(1, y), entireGrid.nx() - 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    // Delete buffers
    delete[] leftReceiveBuffer;
    delete[] rightReceiveBuffer;
    delete[] topReceiveBuffer;
    delete[] bottomReceiveBuffer;
    delete[] leftSendBuffer;
    delete[] rightSendBuffer;
    delete[] topSendBuffer;
    delete[] bottomSendBuffer;
  
    // Stop MPI
    MPI_Finalize();

    return 0;
}
