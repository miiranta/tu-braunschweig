#ifndef MOTOR_H
#define MOTOR_H

void initMotors(void);
void moveForward(int i);
void moveBackward(int i);
void turnRight(int degrees);
void turnLeft(int degrees);
void stop(void);

#endif /* MOTOR_H */
