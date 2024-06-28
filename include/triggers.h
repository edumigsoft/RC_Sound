#ifndef __TRIGGERS_H__
#define __TRIGGERS_H__

#include "variables.h"

void triggerHorn()
{
    static uint32_t bluelightOffDelay = millis();
    if (blueLightTrigger || sirenLatch)
    {
        bluelightOffDelay = millis();
    }

    if (millis() - bluelightOffDelay > 50)
    {
        blueLightTrigger = false;
    }
}

void triggerIndicators()
{
    static boolean L;
    static boolean R;

    if (steeringAngle < STEERING_ANGLE_MIDDLE)
    {
        L = true;
        R = false;
    }
    else if (steeringAngle > STEERING_ANGLE_MIDDLE)
    {
        R = true;
        L = false;
    }
    else if (indicatorLonManual)
    {
        L = true;
        R = false;
    }
    else if (indicatorRonManual)
    {
        L = false;
        R = true;
    }
    else
    {
        L = false;
        R = false;
    }

    if (!INDICATOR_DIR)
    {
        indicatorLon = L;
        indicatorRon = R;
    }
    else
    {
        indicatorLon = R;
        indicatorRon = L;
    }

    if (indicatorLon || indicatorRon)
        hazard = false;
}

#endif // __TRIGGERS_H__