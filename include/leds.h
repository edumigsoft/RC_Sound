#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <statusLED.h>

statusLED headLight(false); // "false" = output not inversed
statusLED tailLight(false);
statusLED indicatorL(false);
statusLED indicatorR(false);
statusLED fogLight(false);
statusLED reversingLight(false);
statusLED roofLight(false);
statusLED sideLight(false);
statusLED beaconLight1(false);
statusLED beaconLight2(false);
statusLED shakerMotor(false);
statusLED cabLight(false);
statusLED brakeLight(false);

void setupStatusLED()
{
    headLight.begin(HEADLIGHT_PIN, 15, 20000);           // Timer 15, 20kHz
    tailLight.begin(TAILLIGHT_PIN, 2, 20000);            // Timer 2, 20kHz
    indicatorL.begin(INDICATOR_LEFT_PIN, 3, 20000);      // Timer 3, 20kHz
    indicatorR.begin(INDICATOR_RIGHT_PIN, 4, 20000);     // Timer 4, 20kHz
    fogLight.begin(FOGLIGHT_PIN, 5, 20000);              // Timer 5, 20kHz
    reversingLight.begin(REVERSING_LIGHT_PIN, 6, 20000); // Timer 6, 20kHz
    roofLight.begin(ROOFLIGHT_PIN, 7, 20000);            // Timer 7, 20kHz
    sideLight.begin(SIDELIGHT_PIN, 8, 20000);            // Timer 8, 20kHz
    beaconLight1.begin(BEACON_LIGHT1_PIN, 9, 20000);     // Timer 9, 20kHz
    beaconLight2.begin(BEACON_LIGHT2_PIN, 10, 20000);    // Timer 10, 20kHz
    brakeLight.begin(BRAKELIGHT_PIN, 11, 20000);         // Timer 11, 20kHz
    cabLight.begin(CABLIGHT_PIN, 12, 20000);             // Timer 12, 20kHz
    shakerMotor.begin(SHAKER_MOTOR_PIN, 13, 20000); // Timer 13, 20kHz
}

#endif // __OBJECTS_H__