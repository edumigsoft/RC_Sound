#ifndef __STEERING_H__
#define __STEERING_H__

#include <ESP32Servo.h>
#include "variables.h"

Servo steeringAngleServo;

void steeringOutput()
{
    int16_t angle;
    int16_t pos4;
    int16_t pos5;

#if STEERING_AFTER == 0
    pos4 = STEERING_ANGLE_RIGHT;
    pos5 = STEERING_ANGLE_LEFT;
#else
    pos4 = STEERING_ANGLE_LEFT;
    pos5 = STEERING_ANGLE_RIGHT;
#endif

    if (steeringValue < 1500)
        angle = map(steeringValue, 1000, 1500, pos4, STEERING_ANGLE_MIDDLE);
    else if (steeringValue > 1500)
        angle = map(steeringValue, 1500, 2000, STEERING_ANGLE_MIDDLE, pos5);
    else
        angle = STEERING_ANGLE_MIDDLE;

    steeringAngleServo.write(angle);
}

void setupSteering()
{
    steeringAngleServo.attach(STEERING_PIN);
    steeringOutput();
}

#endif // __STEERING_H__