#ifndef __TRIGGERS_H__
#define __TRIGGERS_H__

#include "variables.h"

void triggerHorn()
{
    //   if (!winchEnabled && !unlock5thWheel && !hazard)
    //   { // Horn & siren control mode *************
    //     winchPull = false;
    //     winchRelease = false;
    //     // legsUp = false;
    //     // legsDown = false;
    //     // rampsUp = false;
    //     // rampsDown = false;

    //     // detect bluelight trigger ( impulse length < 1300us) ----------
    //     static uint32_t bluelightOffDelay = millis();
    //     if ((pulseWidth[4] < 1300 && pulseWidth[4] > pulseMinLimit[4]) || sirenLatch)
    //     {
    //       bluelightOffDelay = millis();
    //       blueLightTrigger = true;
    //     }
    //     if (millis() - bluelightOffDelay > 50)
    //     { // Switch off delay
    //       blueLightTrigger = false;
    //     }
    //   }

    //   else if (unlock5thWheel)
    //   { // Trailer leg control mode *************************************
    //     winchPull = false;
    //     winchRelease = false;
    //     rampsUp = false;
    //     rampsDown = false;

    //     // legs down ( impulse length > 1900us) -------------
    //     if (pulseWidth[4] > 1900 && pulseWidth[4] < pulseMaxLimit[4])
    //       legsDown = true;
    //     else
    //       legsDown = false;

    //     // legs up ( impulse length < 1100us) -------------
    //     if (pulseWidth[4] < 1100 && pulseWidth[4] > pulseMinLimit[4])
    //       legsUp = true;
    //     else
    //       legsUp = false;
    //   }

    //   else if (hazard)
    //   { // Trailer ramps control mode ***************************************
    //     winchPull = false;
    //     winchRelease = false;
    //     legsUp = false;
    //     legsDown = false;

    //     // ramps down ( impulse length > 1900us) -------------
    //     if (pulseWidth[4] > 1900 && pulseWidth[4] < pulseMaxLimit[4])
    //       rampsDown = true;
    //     else
    //       rampsDown = false;

    //     // ramps up ( impulse length < 1100us) -------------
    //     if (pulseWidth[4] < 1100 && pulseWidth[4] > pulseMinLimit[4])
    //       rampsUp = true;
    //     else
    //       rampsUp = false;
    //   }

    //   else
    //   { // Winch control mode *****************************************************************
    //     legsUp = false;
    //     legsDown = false;
    //     rampsUp = false;
    //     rampsDown = false;

    //     // pull winch ( impulse length > 1900us) -------------
    //     if (pulseWidth[4] > 1900 && pulseWidth[4] < pulseMaxLimit[4])
    //       winchPull = true;
    //     else
    //       winchPull = false;

    //     // release winch ( impulse length < 1100us) -------------
    //     if (pulseWidth[4] < 1100 && pulseWidth[4] > pulseMinLimit[4])
    //       winchRelease = true;
    //     else
    //       winchRelease = false;
    //   }
}

void triggerIndicators()
{
    // #if not defined EXCAVATOR_MODE // Only used, if our vehicle is not an excavator!

    //     static boolean L;
    //     static boolean R;

    // #ifdef AUTO_INDICATORS // Automatic, steering triggered indicators ********
    //     // detect left indicator trigger -------------
    //     if (pulseWidth[1] > (1500 + indicatorOn))
    //     {
    //         L = true;
    //         R = false;
    //     }
    //     if (pulseWidth[1] < (1500 + indicatorOn / 3))
    //         L = false;

    //     // detect right indicator trigger -------------
    //     if (pulseWidth[1] < (1500 - indicatorOn))
    //     {
    //         R = true;
    //         L = false;
    //     }
    //     if (pulseWidth[1] > (1500 - indicatorOn / 3))
    //         R = false;
    // #else // Manually triggered indicators ********
    //     // detect left indicator trigger -------------
    //     // if (pulseWidth[6] > 1900)
    //     if (indicatorLon)
    //     {
    //         L = true;
    //         R = false;
    //     }
    //     // if (pulseWidth[6] < (1500 - indicatorOn / 3))
    //     if (!indicatorLon)
    //         L = false;

    //     // detect right indicator trigger -------------
    //     // if (pulseWidth[6] < 1100)
    //     if (indicatorRon)
    //     {
    //         R = true;
    //         L = false;
    //     }
    //     // if (pulseWidth[6] > (1500 + indicatorOn / 3))
    //     if (!indicatorRon)
    //         R = false;

    //     // Reset by steering -------------
    //     static int steeringOld;
    //     Serial.print("pulseWidth[1] = ");
    //     Serial.print(pulseWidth[1]);
    //     Serial.print(", steeringOld = ");
    //     Serial.println(steeringOld);

    //     if (pulseWidth[1] < steeringOld - 50)
    //     {
    //         L = false;
    //         steeringOld = pulseWidth[1];
    //     }

    //     if (pulseWidth[1] > steeringOld + 50)
    //     {
    //         R = false;
    //         steeringOld = pulseWidth[1];
    //     }

    // #endif // End of manually triggered indicators

    //     // Indicator direction
    //     if (!INDICATOR_DIR)
    //     {
    //         indicatorLon = L;
    //         indicatorRon = R;
    //     }
    //     else
    //     {
    //         indicatorLon = R;
    //         indicatorRon = L;
    //     }

    //     if (indicatorLon || indicatorRon)
    //         hazard = false;

    // #endif
}

#endif // __TRIGGERS_H__