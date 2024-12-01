#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <mpi.h>
#include <cmath>

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

    // Initialize simulation parameters
    constexpr SimulationParameter parameter;

    // Initialize (grid) map
    constexpr Mapper2D innerGrid(parameter.gridNx, parameter.gridNy);
    std::cout << "Grid size: " << innerGrid.nx() << "x" << innerGrid.ny() << std::endl;

    // Initialize (grid + ghost Layers) map
    constexpr Mapper2D entireGrid(innerGrid.nx() + 2, innerGrid.ny() + 2);

    // Initialize 2 grids
    // Now we can apply the calculations more easily
    std::vector<double> oldData (entireGrid.size());
    std::vector<double> newData (entireGrid.size());

    // -------------------------------------------------------------------------------

    // Set all values to 0
    // Avoiding trash that could be on the memory
    for (size_t i = 0; i < entireGrid.size(); i++)
    {
        oldData[i] = 0.0;
        newData[i] = 0.0;
    }

    // Set values of the ghost layers with the parameters we want to simulate
    for (size_t y = 0; y < entireGrid.ny(); y++)
    {
        oldData[entireGrid.pos(0, y)] = parameter.bcLeft;
        oldData[entireGrid.pos(entireGrid.nx() - 1, y)] = parameter.bcRight;

        newData[entireGrid.pos(0, y)] = parameter.bcLeft;
        newData[entireGrid.pos(entireGrid.nx() - 1, y)] = parameter.bcRight;
    }

    for (size_t x = 0; x < entireGrid.nx(); x++)
    {
        oldData[entireGrid.pos(x, 0)] = parameter.bcTop;
        oldData[entireGrid.pos(x, entireGrid.ny() - 1)] = parameter.bcBottom;

        newData[entireGrid.pos(x, 0)] = parameter.bcTop;
        newData[entireGrid.pos(x, entireGrid.ny() - 1)] = parameter.bcBottom;
    }

    // Start timer
    timer.startNupsTimer();
    
    // Iterate
    while (!done)
    {

        // For every slot, evaluate
        // for (size_t i = 0; i < innerGrid.size(); i++) 
        // {
        //     const size_t x = innerGrid.xForPos(i) + 1;
        //     const size_t y = innerGrid.yForPos(i) + 1;

        //     newData[entireGrid.pos(x, y)] = 0.25 * (oldData[entireGrid.pos(x - 1, y)] +
        //                                             oldData[entireGrid.pos(x + 1, y)] +
        //                                             oldData[entireGrid.pos(x, y - 1)] +
        //                                             oldData[entireGrid.pos(x, y + 1)]);
        // }

        // For every slot, evaluate
        // More memory efficient code
        for (size_t y = 1; y < entireGrid.ny() - 1; y++)
        {
            for (size_t x = 1; x < entireGrid.nx() - 1; x++)
            {
                
                newData[entireGrid.pos(x, y)] = 0.25 * (oldData[entireGrid.pos(x - 1, y)] +
                                                        oldData[entireGrid.pos(x + 1, y)] +
                                                        oldData[entireGrid.pos(x, y - 1)] +
                                                        oldData[entireGrid.pos(x, y + 1)]);
                
            }
            
        }

        // Every 'outputInterval', print and evaluate stopping criteria
        iteration++;
        if ((iteration % parameter.outputInterval) == 0) 
        {
            const double error = calcError(oldData, newData, entireGrid);

            // Stopping criteria
            done = (error < parameter.maxError) || (iteration >= parameter.maxIterations);

            // Print
            const size_t mnups = timer.getMNups(long(innerGrid.size() * parameter.outputInterval));
            std::cout << "time step: " << iteration << " error: " << error << " MNUPS: " << mnups << "\n";
            timer.startNupsTimer();
        }

        // Copy new data to old data
        std::swap(newData, oldData);
    }
    
    // Print result grid
    // for (size_t i = 0; i < entireGrid.size(); i++){
    //     if (i % entireGrid.nx() == 0){std::cout << "\n";}
    //     if (oldData[i] < 10){std::cout << oldData[i] << "  ";}
    //     else{std::cout << oldData[i] << " ";}
    // }
    
    // Print result stats
    const auto runtime = timer.getRuntimeSeconds();
    std::cout << "\n";
    std::cout << "Runtime: " << runtime << " s. " << std::endl;
    std::cout << "Average MNUPS: " << timer.getAverageMNups(innerGrid.size() * iteration) << std::endl;

    // Write paraview file
    writeUCDFile("inp/inp_SerialOut.inp", oldData, entireGrid);

    return 0;
}
