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

    // detect right indicator trigger -------------
    if ((steeringValue > (1500 + indicatorOn)) || indicatorRonManual)
    {
        L = false;
        R = true;

        indicatorLonManual = false;
    }
    if ((steeringValue < (1500 + indicatorOn / 3)) & !indicatorRonManual)
        R = false;

    // detect left indicator trigger -------------
    if ((steeringValue < (1500 - indicatorOn)) || indicatorLonManual)
    {
        R = false;
        L = true;

        indicatorRonManual = false;
    }
    if ((steeringValue > (1500 - indicatorOn / 3)) & !indicatorLonManual)
        L = false;

    indicatorLon = L;
    indicatorRon = R;

    if (indicatorLon || indicatorRon)
        hazard = false;
}

#endif // __TRIGGERS_H__