#include "motor.h"

#include "board.h"
#include "ztimer.h"

#include "periph/gpio.h"
#include "periph/adc.h"

#include "../ultrassonic/ultrassonic.h"

#define TURN_CALIBRATION_FACTOR 4

// Motor 1 (esquerda)
const int motorPin1 = GPIO_PIN(PORT_C, 2);
const int motorPin2 = GPIO_PIN(PORT_C, 8);
// Motor 2 (direita)
const int motorPin3 = GPIO_PIN(PORT_C, 9);
const int motorPin4 = GPIO_PIN(PORT_C, 3);

// Pin functions
void pin_stop(void)
{
    gpio_write(motorPin1, 0);
    gpio_write(motorPin2, 0);
    gpio_write(motorPin3, 0);
    gpio_write(motorPin4, 0);
}

void pin_goStraight(void)
{
    pin_stop();
    gpio_write(motorPin1, 1);
    gpio_write(motorPin3, 1);
}

void pin_goBack(void)
{
    pin_stop();
    gpio_write(motorPin2, 1);
    gpio_write(motorPin4, 1);
}

void pin_turnLeft(void)
{
    pin_stop();
    gpio_write(motorPin2, 1); // direita pra tras
    gpio_write(motorPin3, 1); // esquerda pra frente
}

void pin_turnRight(void)
{
    pin_stop();
    gpio_write(motorPin1, 1); // direita pra frente
    gpio_write(motorPin4, 1); // esquerda pra tras
}

void initMotors(void)
{
    initUltrassonic();

    // Motor pins
    gpio_init(motorPin1, GPIO_OUT);
    gpio_init(motorPin2, GPIO_OUT);
    gpio_init(motorPin3, GPIO_OUT);
    gpio_init(motorPin4, GPIO_OUT);
    pin_stop();
}

// Walk functions
void stop(void)
{
    printf("Stop\n");
    pin_stop();
}

void moveForward(int i)
{
    printf("forward\n");
    
    for (int j = 0; j < i; j++)
    {
        int dis = ultrassonicSensorRead();
        if(dis >= 20)
        {
            pin_goStraight();
        }

        ztimer_sleep(ZTIMER_MSEC, 35);
        pin_stop();
        ztimer_sleep(ZTIMER_MSEC, 60);
    }
}

void moveBackward(int i)
{
    printf("back\n");
    for (int j = 0; j < i; j++)
    {
        pin_goBack();
        ztimer_sleep(ZTIMER_MSEC, 35);
        pin_stop();
        ztimer_sleep(ZTIMER_MSEC, 60);
    }
}

void turnLeft(int degrees)
{
    printf("left\n");
    degrees = (int) (degrees / TURN_CALIBRATION_FACTOR);

    for(int i = 0; i < degrees; i++)
    {
        pin_turnLeft();
        ztimer_sleep(ZTIMER_MSEC, 35);
        pin_stop();
        ztimer_sleep(ZTIMER_MSEC, 60);
    }
}

void turnRight(int degrees)
{
    printf("right\n");
    degrees = (int) (degrees / TURN_CALIBRATION_FACTOR);

    for (int i = 0; i < degrees; i++)
    {
        pin_turnRight();
        ztimer_sleep(ZTIMER_MSEC, 35);
        pin_stop();
        ztimer_sleep(ZTIMER_MSEC, 60);
    }
}
