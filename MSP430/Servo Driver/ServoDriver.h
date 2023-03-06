#include <msp430fr4133.h>


#ifndef SERVO_DRIVER
#define SERVO_DRIVER


void initServo(unsigned int min, unsigned int max);
void setAngle(unsigned int angle);

#endif
