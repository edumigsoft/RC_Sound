#ifndef __STEERING_H__
#define __STEERING_H__

#include <ESP32Servo.h>
#include "variables.h"

Servo steeringAngleServo;

void steeringOutput()
{
    int16_t angle;

    if (steeringValue < 1500)
        angle = map(steeringValue, 1000, 1500, STEERING_ANGLE_LEFT, STEERING_ANGLE_MIDDLE);
    else if (steeringValue > 1500)
        angle = map(steeringValue, 1500, 2000, STEERING_ANGLE_MIDDLE, STEERING_ANGLE_RIGHT);
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