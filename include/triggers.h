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

    // detect left indicator trigger -------------
    if (steeringValue > (1500 + indicatorOn))
    {
        L = true;
        R = false;
    }
    if (steeringValue < (1500 + indicatorOn / 3))
        L = false;

    // detect right indicator trigger -------------
    if (steeringValue < (1500 - indicatorOn))
    {
        R = true;
        L = false;
    }
    if (steeringValue > (1500 - indicatorOn / 3))
        R = false;
        
    // else if (indicatorLonManual)
    if (indicatorLonManual)
    {
        L = true;
        R = false;
    }
    else if (indicatorRonManual)
    {
        L = false;
        R = true;
    }
    // else
    // {
    //     L = false;
    //     R = false;
    // }

    indicatorLon = R;
    indicatorRon = L;

    if (indicatorLon || indicatorRon)
        hazard = false;
}

#endif // __TRIGGERS_H__