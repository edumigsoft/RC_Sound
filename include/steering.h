#ifndef __STEERING_H__
#define __STEERING_H__

#include <ESP32Servo.h>
#include "variables.h"

Servo steeringAngleServo;

void steeringOutput()
{
    steeringAngleServo.write(steeringAngle);
}

void setupSteering()
{
    steeringAngleServo.attach(STEERING_PIN);
    steeringOutput();
}

#endif // __STEERING_H__