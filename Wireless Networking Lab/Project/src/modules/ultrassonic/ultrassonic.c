#include "ultrassonic.h"

#include "board.h"

#include <ztimer.h>

#define THREAD_SIZE 512
char ultrassonicThread_stack[THREAD_SIZE];

// Ultrassonic sensor
const int trigPin = GPIO_PIN(PORT_C, 4);
const int echoPin = GPIO_PIN(PORT_C, 10);

int distance = 0;

void *ultrassonicThread(void *args)
{
    (void)args;

    while (1)
    {
        ztimer_sleep(ZTIMER_MSEC, 2);

        int timeout = 100000; // uS

        gpio_write(trigPin, 0);
        ztimer_sleep(ZTIMER_MSEC, 2);

        gpio_write(trigPin, 1);
        ztimer_sleep(ZTIMER_USEC, 10);
        gpio_write(trigPin, 0);
        while (gpio_read(echoPin) == 0)
        {
            timeout--;
            if (timeout == 0)
            {
                distance = 0;
                break;
            }
        }

        if (timeout == 0)
        {
            continue;
        }

        uint32_t start = ztimer_now(ZTIMER_USEC);
        while (gpio_read(echoPin))
        {
            timeout--;
            if (timeout == 0)
            {
                distance = 200;
                break;
            }
        }

        if (timeout == 0)
        {
            continue;
        }


        uint32_t end = ztimer_now(ZTIMER_USEC);

        int duration = end - start;
        distance = duration * 0.034 / 2;
    }

    return NULL;
}

void initUltrassonic(void)
{
    gpio_init(trigPin, GPIO_OUT);
    gpio_init(echoPin, GPIO_IN);

    gpio_write(trigPin, 0);

    thread_create(
        ultrassonicThread_stack,
        THREAD_SIZE,
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        ultrassonicThread,
        NULL,
        "ultrassonicThread");
}

int ultrassonicSensorRead(void)
{
    return distance;
}
