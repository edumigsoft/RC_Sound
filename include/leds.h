#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <statusLED.h>
#include "6_Lights.h"

statusLED headLight(false); // "false" = output not inversed
// statusLED tailLight(false);
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
    // tailLight.begin(TAILLIGHT_PIN, 2, 20000);            // Timer 2, 20kHz
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
    shakerMotor.begin(SHAKER_MOTOR_PIN, 13, 20000);      // Timer 13, 20kHz
}

void brakeLightsSub(uint8_t brightness)
{
    if (escIsBraking)
    {
        // tailLight.pwm(255 - crankingDim);  // Taillights (full brightness)
        brakeLight.pwm(255 - crankingDim); // Brakelight on
    }
    else
    {
        // tailLight.pwm(constrain(brightness - (crankingDim / 2), (brightness / 2), 255));  // Taillights (reduced brightness)
        brakeLight.pwm(constrain(brightness - (crankingDim / 2), (brightness / 2), 255)); // Brakelight (reduced brightness)
    }
}

void headLightsSub(bool head, bool fog, bool roof, bool park)
{
    fogLightOn = fog;

#ifdef XENON_LIGHTS // Optional xenon ignition flash
    if (millis() - xenonMillis > 50)
        xenonIgnitionFlash = 0;
    else
        xenonIgnitionFlash = 170; // bulb is brighter for 50ms
#endif

#ifdef SEPARATE_FULL_BEAM // separate full beam bulb, wired to "rooflight" pin ----
    // Headlights (low beam bulb)
    if (!head && !park)
    {
        headLight.off();
        xenonMillis = millis();
        if (!headLightsFlasherOn)
            headLightsHighBeamOn = false;
    }
    else if (park)
    { // Parking lights
        headLight.pwm(constrain(headlightParkingBrightness - crankingDim, (headlightParkingBrightness / 2), 255));
        xenonMillis = millis();
        if (!headLightsFlasherOn)
            headLightsHighBeamOn = false;
    }
    else
    { // ON
        headLight.pwm(constrain(255 - crankingDim - 170 + xenonIgnitionFlash, 0, 255));
    }
    // Headlights (high beam bulb)
    if (headLightsFlasherOn || (headLightsHighBeamOn && head))
        roofLight.pwm(200 - crankingDim);
    else
        roofLight.off();

#else  // Bulbs wired as labeled on the board ----
    // Headlights
    if (!head && !park)
    { // OFF or flasher
        if (!headLightsFlasherOn)
            headLight.off();
        else
            headLight.on();
        xenonMillis = millis();
        headLightsHighBeamOn = false;
    }
    else if (park)
    { // Parking lights
        if (!headLightsFlasherOn)
            headLight.pwm(constrain(headlightParkingBrightness - crankingDim, (headlightParkingBrightness / 2), 255));
        else
            headLight.on();
        xenonMillis = millis();
        headLightsHighBeamOn = false;
    }
    else
    { // ON
        headLight.pwm(constrain(255 - crankingDim - dipDim + xenonIgnitionFlash, 0, 255));
    }

    // Roof lights
    if (!roof)
        roofLight.off();
    else
        roofLight.pwm(130 - crankingDim);
#endif // ----

    // Fog lights
    if (!fog)
        fogLight.off();
    else
        fogLight.pwm(200 - crankingDim);
}

void led()
{
#if defined LED_INDICATORS
    indicatorFade = 0; // No soft indicator on / off, if LED
#endif

    // Lights brightness ----
#if defined FLICKERING_WHILE_CRANKING
    static unsigned long flickerMillis;
    if (millis() - flickerMillis > 30)
    { // Every 30ms
        flickerMillis = millis();
        if (engineStart)
            crankingDim = random(25, 55);
        else
            crankingDim = 0; // lights are dimmer and flickering while engine cranking
    }
#else
    if (engineStart)
        crankingDim = 50;
    else
        crankingDim = 0; // lights are dimmer while engine cranking
#endif

    if (headLightsFlasherOn || headLightsHighBeamOn)
        dipDim = 10;
    else
        dipDim = 170; // High / low beam and headlight flasher (SBUS CH5)

    // Reversing light ----
    if ((engineRunning || engineStart) && escInReverse)
        reversingLight.pwm(reversingLightBrightness - crankingDim);
    else
        reversingLight.off();

    if (blueLightTrigger)
    {
        if (doubleFlashBlueLight)
        {
            beaconLight1.flash(30, 80, 400, 2);      // Simulate double flash lights
            beaconLight2.flash(30, 80, 400, 2, 330); // Simulate double flash lights (with delay for first pass)
        }
        else
        {
            beaconLight1.flash(30, 500, 0, 0);      // Simulate rotating beacon lights with short flashes
            beaconLight2.flash(30, 500, 0, 0, 100); // Simulate rotating beacon lights with short flashes
        }
    }
    else
    {
        beaconLight2.off();
        beaconLight1.off();
    }

    // Indicators (turn signals, blinkers) ----
    uint8_t indicatorOffBrightness;
#if defined INDICATOR_SIDE_MARKERS // Indicators used as US style side markers as well
    if (lightsState > 1)
        indicatorOffBrightness = rearlightDimmedBrightness - crankingDim / 2;
    else
        indicatorOffBrightness = 0;
#else
    indicatorOffBrightness = 0;
#endif

#ifdef HAZARDS_WHILE_5TH_WHEEL_UNLOCKED
    if (!hazard && !unlock5thWheel && !batteryProtection)
    { // Hazards also active, if 5th wheel unlocked
#else
    if (!hazard && !batteryProtection)
    {
#endif
        if (indicatorLon)
        {
            if (indicatorL.flash(375, 375, 0, 0, 0, indicatorFade, indicatorOffBrightness))
                indicatorSoundOn = true; // Left indicator
        }
#if defined INDICATOR_SIDE_MARKERS // Indicators used as US style side markers as well
        else
        {
            if (lightsState > 1)
                indicatorL.pwm(rearlightDimmedBrightness - crankingDim / 2);
            else
                indicatorL.off(indicatorFade);
        }
#else
        else
            indicatorL.off(indicatorFade);
#endif

        if (indicatorRon)
        {
            if (indicatorR.flash(375, 375, 0, 0, 0, indicatorFade, indicatorOffBrightness))
                indicatorSoundOn = true; // Left indicator
        }
#if defined INDICATOR_SIDE_MARKERS // Indicators used as US style side markers as well
        else
        {
            if (lightsState > 1)
                indicatorR.pwm(rearlightDimmedBrightness - crankingDim / 2);
            else
                indicatorR.off(indicatorFade);
        }
#else
        else
            indicatorR.off(indicatorFade);
#endif
    }
    else
    { // Hazard lights on, if no connection to transmitter (serial & SBUS control mode only)
        if (indicatorL.flash(375, 375, 0, 0, 0, indicatorFade, indicatorOffBrightness))
            indicatorSoundOn = true;
        indicatorR.flash(375, 375, 0, 0, 0, indicatorFade, indicatorOffBrightness);
    }

    // Headlights, tail lights ----
#ifdef AUTO_LIGHTS // automatic lights mode (deprecated, not maintained anymore!) ************************

#ifdef XENON_LIGHTS // Optional xenon ignition flash
    if (millis() - xenonMillis > 50)
        xenonIgnitionFlash = 0;
    else
        xenonIgnitionFlash = 170; // bulb is brighter for 50ms
#endif
    if (lightsOn && (engineRunning || engineStart))
    {
        headLight.pwm(constrain(255 - crankingDim - dipDim + xenonIgnitionFlash, 0, 255));
        brakeLightsSub(rearlightDimmedBrightness);
    }

    else
    {
        headLight.off();
        tailLight.off();
        brakeLight.off();
        xenonMillis = millis();
        headLightsHighBeamOn = false;
    }

    // Foglights ----
    if (lightsOn && engineRunning)
    {
        fogLight.pwm(200 - crankingDim);
        fogLightOn = true;
    }
    else
    {
        fogLight.off();
        fogLightOn = false;
    }

    // Roof lights ----
    if (lightsOn)
        roofLight.pwm(130 - crankingDim);
    else
        roofLight.off();

    // Sidelights ----
    if (engineOn)
        sideLight.pwm(200 - crankingDim);
    else
        sideLight.off();

    // Cabin lights ----
    if (!lightsOn)
        cabLight.pwm(255 - crankingDim);
    else
        cabLight.off();

#else // manual lights mode ************************
    // Lights state machine
    switch (lightsState)
    {

    case 0: // lights off ---------------------------------------------------------------------
        cabLight.off();
        sideLight.off();
        lightsOn = false;
        headLightsSub(false, false, false, false);
        brakeLightsSub(0); // 0 brightness, if not braking
        break;

    case 1: // cab lights ---------------------------------------------------------------------
#ifdef NO_CABLIGHTS
        lightsState = 2; // Skip cablights
#else
        cabLight.pwm(cabLightsBrightness - crankingDim);
#endif
        sideLight.off();
        headLightsSub(false, false, false, false);
        brakeLightsSub(0); // 0 brightness, if not braking
        break;

    case 2: // cab & roof & side lights ---------------------------------------------------------------------
#ifndef NO_CABLIGHTS
        cabLight.pwm(cabLightsBrightness - crankingDim);
#endif
        sideLight.pwm(constrain(sideLightsBrightness - crankingDim, (sideLightsBrightness / 2), 255));
        headLightsSub(false, false, true, true);
        fogLight.off();
        brakeLightsSub(rearlightParkingBrightness); // () = brightness, if not braking
        break;

    case 3: // roof & side & head lights ---------------------------------------------------------------------
        cabLight.off();
        sideLight.pwm(constrain(sideLightsBrightness - crankingDim, (sideLightsBrightness / 2), 255));
        lightsOn = true;
        headLightsSub(true, false, true, false);
        brakeLightsSub(rearlightDimmedBrightness); // 50 brightness, if not braking
        break;

    case 4: // roof & side & head & fog lights ---------------------------------------------------------------------
#ifdef NO_FOGLIGHTS
        lightsState = 5; // Skip foglights
#endif
        cabLight.off();
        sideLight.pwm(constrain(sideLightsBrightness - crankingDim, (sideLightsBrightness / 2), 255));
        headLightsSub(true, true, true, false);
        brakeLightsSub(rearlightDimmedBrightness); // 50 brightness, if not braking
        break;

    case 5: // cab & roof & side & head & fog lights ---------------------------------------------------------------------
#ifdef NO_CABLIGHTS
        lightsState = 0; // Skip cablights
#endif
        cabLight.pwm(cabLightsBrightness - crankingDim);
        sideLight.pwm(constrain(sideLightsBrightness - crankingDim, (sideLightsBrightness / 2), 255));
        headLightsSub(true, true, true, false);
        brakeLightsSub(rearlightDimmedBrightness); // 50 brightness, if not braking
        break;

    } // End of state machine
#endif // End of manual lights mode ************************
}

#endif // __OBJECTS_H__