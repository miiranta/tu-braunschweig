#ifndef STATES_H
#define STATES_H

typedef enum States
{
    INITIALIZING,
    DISCOVERING,
    BUILDING_PATH,
    COMMUNICATING,
    MOVING,
    HALT
} States;

#endif /* STATES_H */
