#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ztimer.h>
#include <math.h>

#include "board.h"

#include "net/gnrc.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/udp.h"
#include "net/ipv6/addr.h"
#include "net/netif.h"

#include "modules/msp/msp.h"
#include "modules/enums/states.h"
#include "modules/triggerTimer/triggerTimer.h"
#include "modules/motor/motor.h"
#include "modules/pathGenerator/pathGenerator.h"

#define THREAD_STACKSIZE 512
#define MAX_HISTORY_SIZE 25
#define FORWARD_AMOUNT 40
#define TURN_DEGREES 90

// Conditions
bool isLeader = true;
bool isConflictSolved = false;

// States
char commandsHistory[MAX_HISTORY_SIZE];
States currentState = INITIALIZING;

// Car variables
int id = 1;
int pathSize = -1;

// Network variables
bool cars[16] = {false};
int pathSizesReceived = 0;
int smallestPathSize = INT_MAX;
int smallestPathId = 0;
int numCars = 0;

// Threads
kernel_pid_t collisionTrd;

// IPv6
ipv6_addr_t my_addr[2];
char my_ipv6_0[IPV6_ADDR_MAX_STR_LEN];
char my_ipv6_1[IPV6_ADDR_MAX_STR_LEN];

//Leds
void update_leds(void) 
{
    LED0_OFF;
    LED1_OFF;
    LED2_OFF;
    LED3_OFF;

    // Check if first bit is 1
    if (id & (1 << 0))
    {
        LED0_ON;
    }

    if (id & (1 << 1))
    {
        LED1_ON;
    }

    if (id & (1 << 2))
    {
        LED2_ON;
    }

    if (id & (1 << 3))
    {
        LED3_ON;
    }
}

void timer_cb(void)
{
    printf("Timer callback\n");

    currentState = BUILDING_PATH;

    if (isLeader)
    {
        announceCars(numCars);
    }
}

void print_my_ipv6(void)
{
    printf("My IPv6 (0): %s\n", my_ipv6_0);
    printf("My IPv6 (1): %s\n", my_ipv6_1);
}

void init(void)
{
    //Get IPv6
    netifs_get_ipv6(my_addr, 2);

    ipv6_addr_to_str(my_ipv6_0, &my_addr[0], IPV6_ADDR_MAX_STR_LEN);
    ipv6_addr_to_str(my_ipv6_1, &my_addr[1], IPV6_ADDR_MAX_STR_LEN);

    print_my_ipv6();

    currentState = DISCOVERING;

    // Send the car id to the network
    handshake(id);
    update_leds();

    // Start the timer thread to wait all cars initialize in the network
    start_timer(&numCars);
    register_timer_callback(timer_cb);
}

//Control
void startMoving(void)
{
    currentState = MOVING;
}

void *randomPathTrd(void *args)
{
    (void)args;

    pathSize = findPathToGoal(commandsHistory, MAX_HISTORY_SIZE);

    currentState = COMMUNICATING;

    sendPathSize(pathSize);

    printf("Waiting for %d cars\n", numCars);
    while (pathSizesReceived < numCars)
    {
        ztimer_sleep(ZTIMER_MSEC, 50);
    }
    printf("All cars have sent their path sizes\n");

#ifdef DEBUG
    printf("My path size: %d | Smallest path size: %d", pathSize, smallestPathSize);
    printf(" | My ID: %d | Smallest path size ID: %d\n", id, smallestPathId);
#endif
    
    if (pathSize < smallestPathSize 
        || (pathSize == smallestPathSize && id < smallestPathId)) 
    {
        smallestPathSize = pathSize;
        smallestPathId = id;

        waitForConfirmation(sendPath(commandsHistory, pathSize), numCars - 1, 1000);

        printf("SENDER: Walking\n");
        sendGo();
        startMoving();
    }

    return NULL;
}

void receive_size_cb(int size, int senderId)
{
    printf("Received path size: %d\n", size);

    pathSizesReceived++;
    
    if (size < smallestPathSize 
        || (size == smallestPathSize && senderId < id))
    {
        smallestPathSize = size;
        smallestPathId = senderId;
    }
}

void receive_path_cb(Commands *path, int pathSize)
{
    printf("Received path (%d): ", pathSize);
    
    int i;

    for (i = 0; i < pathSize; i++)
    {
        commandsHistory[i] = path[i] + '0';
        printf("%d ", path[i]);
    }

    for (; i < MAX_HISTORY_SIZE; i++)
    {
        commandsHistory[i] = 0;
    }

    printf("\n");
}

void *handleIdColision(void *args)
{
    (void)args;

    // For some reason, some other car has the same id as this one
    // so we solve this problem by taking the next available id and
    // announcing it again

    while (1)
    {
        printf("Collision detected, changing id\n");

        srand(ztimer_now(ZTIMER_USEC));
        int randomTime = rand() % 100 * 10;
        printf("Waiting %d ms\n", randomTime);
        ztimer_sleep(ZTIMER_MSEC, randomTime);

        if (!isConflictSolved)
        {
            printf("Solving id conflict\n");
            for (int i = id + 1; i < 16; i++)
            {
                if (!cars[i])
                {
                    printf("Setting %d as new id instead of %d\n", i, id);

                    solveIdConflict(id, i);
                    id = i;
                    update_leds();

                    break;
                }
            }
        }
        else
        {
            printf("Id conflict solved by the other side\n");
        }

        thread_sleep();
    }

    return NULL;
}

void handshake_cb(int receivedId)
{
    printf("Received id %d\n", receivedId);

    if (!cars[receivedId])
    {
        numCars++;
    }

    cars[receivedId] = true;

    if (currentState == INITIALIZING)
    {
        id++;
    }
    else if (currentState == DISCOVERING && receivedId == id)
    {
        isConflictSolved = false;
        thread_wakeup(collisionTrd);
    }
}

void carsAnnouncement_cb(int finalNumCars)
{
    printf("A car waited and did not receive more handshakes\n");
    isLeader = false;
    numCars = finalNumCars;
    stop_timer();
}

void go_cb(void)
{
    // start walk
    printf("RECEIVER: Walking\n");
    startMoving();
}

void *motorTrd(void *args)
{
    (void)args;

    for (uint8_t i = 0; i < id; i++)
    {
        moveForward(FORWARD_AMOUNT);
    }

    printf("Walking for %d steps\n", smallestPathSize);
    for (int i = 0; i < smallestPathSize; i++)
    {
        Commands currentCommand = (Commands)(commandsHistory[i] - '0');
        sendCommand(currentCommand);

        switch (currentCommand)
        {
        case FORWARD:
            moveForward(FORWARD_AMOUNT);
            break;
        case TURN_RIGHT:
            turnRight(TURN_DEGREES);
            ztimer_sleep(ZTIMER_MSEC, 500);
            moveForward(FORWARD_AMOUNT);
            break;
        case TURN_LEFT:
            turnLeft(TURN_DEGREES);
            ztimer_sleep(ZTIMER_MSEC, 500);
            moveForward(FORWARD_AMOUNT);
            break;
        case STOP:
            stop();
            break;
        }

        ztimer_sleep(ZTIMER_MSEC, 500);
    }

    for (uint8_t i = numCars; i > id; i--)
    {
        moveForward(FORWARD_AMOUNT);
    }

    currentState = HALT;

    return NULL;
}

void idConflict_cb(int oldId, int newId)
{
    printf("Id conflict detected: %d -> %d\n", oldId, newId);

    if (oldId == id)
    {
        printf("The other car has solved the conflict\n");
        isConflictSolved = true;
        handshake(id);
    }
    else
    {
        printf("The conflict was between other two cars\n");
    }

    cars[newId] = true;

    printf("I knew %d cars\n", numCars);
    numCars = 0;
    for (int i = 1; i < 16; i++)
    {
        if (i == id)
        {
            continue;
        }

        if (cars[i])
        {
            numCars++;
        }
    }
    printf("Now I know %d cars\n", numCars);
}

void initMSP(void)
{
    start_msp_server();

    register_go_callback(go_cb);
    register_handshake_callback(handshake_cb);
    register_id_conflict_callback(idConflict_cb);
    register_receive_path_callback(receive_path_cb);
    register_receive_path_size_callback(receive_size_cb);
    register_cars_announcement_callback(carsAnnouncement_cb);
}

int main(void)
{
    // Thread stacks
    char trd_stack[THREAD_STACKSIZE];
    char aux_stack[THREAD_STACKSIZE];

    cars[0] = true;

    ztimer_sleep(ZTIMER_MSEC, 1000);
    printf("Starting!\n");

    init();
    initMotors();
    initMSP();

    collisionTrd = thread_create(aux_stack, sizeof(aux_stack), THREAD_PRIORITY_MAIN - 3, THREAD_CREATE_SLEEPING, handleIdColision, NULL, "id_collision");

    while (currentState != BUILDING_PATH)
    {
        ztimer_sleep(ZTIMER_MSEC, 100);
    }

    thread_create(trd_stack, sizeof(trd_stack), THREAD_PRIORITY_MAIN - 3, THREAD_CREATE_STACKTEST, randomPathTrd, NULL, "random_path");

    while (currentState != MOVING)
    {
        ztimer_sleep(ZTIMER_MSEC, 100);
    }

    // Wait 20 seconds to start moving
    ztimer_sleep(ZTIMER_MSEC, 20000);

    // Wait (id-1)*500ms to start moving
    if(id > 1) ztimer_sleep(ZTIMER_MSEC, (id - 1) * 5000);
    
    thread_create(trd_stack, sizeof(trd_stack), THREAD_PRIORITY_MAIN - 3, THREAD_CREATE_STACKTEST, motorTrd, NULL, "motor_trd");

    while (currentState != HALT)
    {
        ztimer_sleep(ZTIMER_MSEC, 100);
    }

    printf("Done\n");

    ztimer_sleep(ZTIMER_MSEC, 5000);

    return 0;
}