#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "board.h"

#include "periph/gpio.h"
#include "periph/adc.h"

#include "modules/debug/debug.h"
#include "modules/udp/udp.h"

#include "ztimer.h"

#include "shell.h"

#define THREAD_STACKSIZE 512

int32_t infrared = 0;

//GPIO_PIN(0, 0); PA0
//GPIO_PIN(0, 1); PA1
//GPIO_PIN(0, 1); PB0
//...

//Motor 1
const int motorPin1 = GPIO_PIN(PORT_C, 2);
const int motorPin2 = GPIO_PIN(PORT_C, 8);
//Motor 2
const int motorPin3 = GPIO_PIN(PORT_C, 9);
const int motorPin4 = GPIO_PIN(PORT_C, 3);

void walk(int speed)
{
    gpio_write(motorPin1, speed);
    gpio_write(motorPin3, speed);
}

static void button_cb(void *arg)
{
    (void)arg;
    walk(0);
}

void *infraredTrd(void *arg)
{
    (void)arg;

    adc_init(ADC_LINE(0));

    int32_t x = 0;
    while (1)
    {
        x = 0;
        for (int i = 0; i < 10; i++)
        {
            ztimer_sleep(ZTIMER_MSEC, 10);
            x += adc_sample(ADC_LINE(0), ADC_RES_12BIT);
        }

        infrared = x / 10;

        printf("Infrared: %ld\n", infrared);
    }

    return NULL;
}

void *motorTrd(void *arg)
{
    (void)arg;

    while (1)
    {
        if (infrared > 3000)
        {
            walk(1);
        } else
        {
            walk(0);
        }
    }

    return NULL;
}

int main(void)
{
    // Thread stacks
    char infrared_trd_stack[THREAD_STACKSIZE];
    char motor_trd_stack[THREAD_STACKSIZE];


    ztimer_sleep(ZTIMER_MSEC, 1000);
    printf("Starting\n");
    gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_FALLING, button_cb, NULL);

    //Motor pins
    gpio_init(motorPin1, GPIO_OUT);
    gpio_init(motorPin2, GPIO_OUT);
    gpio_init(motorPin3, GPIO_OUT);
    gpio_init(motorPin4, GPIO_OUT);
    gpio_write(motorPin1, 0);
    gpio_write(motorPin2, 0);
    gpio_write(motorPin3, 0);
    gpio_write(motorPin4, 0);

    /* init infrared thread */
    kernel_pid_t infrared_trd_pid = thread_create(infrared_trd_stack, sizeof(infrared_trd_stack), THREAD_PRIORITY_MAIN + 1, THREAD_CREATE_STACKTEST, infraredTrd, NULL, "infrared_trd");

    kernel_pid_t motor_trd_pid = thread_create(motor_trd_stack, sizeof(motor_trd_stack), THREAD_PRIORITY_MAIN + 1, THREAD_CREATE_STACKTEST, motorTrd, NULL, "motor_trd");

    printf("INFRARED_PID = %d\n", infrared_trd_pid);
    printf("MOTOR_PID = %d\n", motor_trd_pid);
    
    while (1)
    {
        ztimer_sleep(ZTIMER_MSEC, 1000);
    }
}
