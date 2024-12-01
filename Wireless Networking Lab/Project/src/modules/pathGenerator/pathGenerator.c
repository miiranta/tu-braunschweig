#include "pathGenerator.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ztimer.h>

#include "../enums/directions.h"
#include "../enums/commands.h"

const uint8_t final_x = 3;
const uint8_t final_y = 3;
const uint8_t max_x = 5;
const uint8_t max_y = 5;
const uint8_t min_x = 0;
const uint8_t min_y = 0;

Directions updateDirection(Directions currentDirection, Commands command)
{
    if (command == TURN_RIGHT)
    {
        return (Directions)((currentDirection + 1) % 4);
    }
    else if (command == TURN_LEFT)
    {
        return (Directions)((currentDirection + 3) % 4);
    }

    return currentDirection;
}

bool move(Directions direction, uint8_t *current_x, uint8_t *current_y)
{
    switch (direction)
    {
    case UP:
        if (*current_y == max_y)
        {
            return false;
        }

        ++(*current_y);
        break;

    case RIGHT:
        if (*current_x == max_x)
        {
            return false;
        }

        ++(*current_x);
        break;

    case DOWN:
        if (*current_y == min_y)
        {
            return false;
        }

        --(*current_y);
        break;

    case LEFT:
        if (*current_x == min_x)
        {
            return false;
        }

        --(*current_x);
        break;

    default:
        return false;
        break;
    }

    (void)*current_x;
    (void)*current_y;

    return true;
}

uint8_t findPathToGoal(uint8_t *commandsHistory, uint8_t commandsHistorySize)
{
    srand(ztimer_now(ZTIMER_MSEC));

    uint8_t i = 0;
    uint8_t current_x = 0, current_y = 0;

    while (current_x != final_x || current_y != final_y)
    {
        i = 0;
        current_x = 0;
        current_y = 0;
        Directions currentDirection = UP;

        while (i < commandsHistorySize && (current_x != final_x || current_y != final_y))
        {
            Commands randomCommand = (Commands)(rand() % 3);
            Directions newDirection = updateDirection(currentDirection, randomCommand);

            if (move(newDirection, &current_x, &current_y))
            {
                currentDirection = newDirection;
                commandsHistory[i] = randomCommand;
                i++;
            }
        }
    }

    printf("Reached the goal in %d steps\n", i);

    return i;
}