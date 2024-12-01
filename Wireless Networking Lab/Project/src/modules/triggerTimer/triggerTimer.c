#include "triggerTimer.h"

#include "ztimer.h"

#include <stdio.h>

void (*timerCallback)(void) = NULL;

static char timer_stack[512];

int8_t attempts = 0;
int8_t maxNumAttempts = 3;
uint16_t waitTime = 3000;
bool kill = false;

bool wait(void)
{
    const int timePartitions = 100;

    for (int i = 0; i < timePartitions; i++)
    {
        ztimer_sleep(ZTIMER_MSEC, waitTime / timePartitions);
        
        if (kill)
        {
            return false;
        }
    }

    return true;
}

void *timerTrdFunc(void* arg)
{
    int* carsCount = (int*)arg;
    uint8_t lastCarCount = *carsCount;
    
    while (attempts < maxNumAttempts)
    {
        if (!wait())
        {
            break;
        }

        if (*carsCount == lastCarCount)
        {
            attempts++;
            printf("Attempt %d\n", attempts);
        }
        else
        {
            lastCarCount = *carsCount;
            attempts = 0;
        }
    }

    if (timerCallback != NULL)
    {
        timerCallback();
    }

    return NULL;
}

bool hasExecuted = false;
void start_timer(int* carsCount)
{
    if (hasExecuted)
    {
        return;
    }

    hasExecuted = true;

    thread_create(
        timer_stack,
        sizeof(timer_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        timerTrdFunc,
        (void*)carsCount,
        "timer_thread"
    );
}

void stop_timer(void)
{
    kill = true;
}

void register_timer_callback(void (*timer_cb)(void))
{
    timerCallback = timer_cb;
}

