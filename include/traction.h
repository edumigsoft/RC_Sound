#ifndef __TRACTION_H__
#define __TRACTION_H__

#include <Arduino.h>
#include "variables.h"
#include "3_ESC.h"
#include "8_Sound.h"

#include <ESP32MX1508.h>

// // Optional Parameters
// #define RES 8     // Resolution in bits:  8 (0-255),  12 (0-4095), or 16 (0-65535)
// #define FREQ 5000 // PWM Frequency in Hz

MX1508 motorTraction(TRACTION_PIN_1, TRACTION_PIN_2, TRACTION_CHANNEL_1, TRACTION_CHANNEL_2); // Default-  8 bit resoluion at 2500 Hz
// // MX1508 motorA(PINA,PINB, CH1, CH2, RES);                // Specify resolution
// // MX1508 motorA(PINA,PINB, CH1, CH2, RES, FREQ);          // Specify resolution and frequency

int8_t pulse()
{ // Throttle direction
    int8_t pulse;
    if (tractionValue > pulseMaxNeutral3 && tractionValue < pulseMaxLimit3)
        pulse = 1; // 1 = Forward
    else if (tractionValue < pulseMinNeutral3 && tractionValue > pulseMinLimit3)
        pulse = -1; // -1 = Backwards
    else
        pulse = 0; // 0 = Neutral
    return pulse;
    // return pulseNew;
}

int8_t tractionPulse()
{
    int8_t escPulse;
    if (escPulseWidth > pulseMaxNeutral3 && escPulseWidth < pulseMaxLimit3)
        escPulse = 1; // 1 = Forward
    else if (escPulseWidth < pulseMinNeutral3 && escPulseWidth > pulseMinLimit3)
        escPulse = -1; // -1 = Backwards
    else
        escPulse = 0; // 0 = Neutral
    return escPulse;
}

// If you connect your ESC to pin 33, the vehicle inertia is simulated. Direct brake (crawler) ESC required
// *** WARNING!! Do it at your own risk!! There is a falisafe function in case, the signal input from the
// receiver is lost, but if the ESP32 crashes, the vehicle could get out of control!! ***

void tractionOutput()
{
// Battery protection --------------------------------
#if defined BATTERY_PROTECTION
    static unsigned long lastBatteryTime;
    static bool outOfFuelMessageLock;
    if (millis() - lastBatteryTime > 300)
    { // Check battery voltage every 300ms
        lastBatteryTime = millis();
        batteryVoltage = batteryVolts(); // Store voltage in global variable (also used in dashboard)
        if (batteryVoltage < batteryCutoffvoltage)
        {
            Serial.printf("Battery protection triggered, slowing down! Battery: %.2f V Threshold: %.2f V \n", batteryVoltage, batteryCutoffvoltage);
            Serial.printf("Disconnect battery to prevent it from overdischarging!\n", batteryVoltage, batteryCutoffvoltage);
            batteryProtection = true;
        }
        if (batteryVoltage > batteryCutoffvoltage + (FULLY_CHARGED_VOLTAGE * numberOfCells))
        { // Recovery hysteresis
            batteryProtection = false;
        }
        // Out of fuel message triggering
        if (batteryProtection && !outOfFuelMessageLock)
        {
            outOfFuelMessageTrigger = true;
            outOfFuelMessageLock = true; // only trigger message once
        }
    }
#endif // --------------------------------------------

#if not defined TRACKED_MODE && not defined AIRPLANE_MODE // No ESC control in TRACKED_MODE or in AIRPLANE_MODE
                                                          // Gear dependent ramp speed for acceleration & deceleration
#if defined VIRTUAL_3_SPEED
    escRampTime = escRampTimeThirdGear * 10 / virtualManualGearRatio[selectedGear];

#elif defined VIRTUAL_16_SPEED_SEQUENTIAL
    escRampTime = escRampTimeThirdGear * virtualManualGearRatio[selectedGear] / 5;

#elif defined STEAM_LOCOMOTIVE_MODE
    escRampTime = escRampTimeSecondGear;

#else // TAMIYA 3 speed shifting transmission
    if (selectedGear == 1)
        escRampTime = escRampTimeFirstGear; // about 20
    if (selectedGear == 2)
        escRampTime = escRampTimeSecondGear; // about 50
    if (selectedGear == 3)
        escRampTime = escRampTimeThirdGear; // about 75
#endif

    if (automatic || doubleClutch)
    {
        escRampTime = escRampTimeSecondGear; // always use 2nd gear acceleration for automatic transmissions
        if (escInReverse)
            escRampTime = escRampTime * 100 / automaticReverseAccelerationPercentage; // faster acceleration in automatic reverse, EXPERIMENTAL, TODO!
    }

    // Allows to scale vehicle file dependent acceleration
    escRampTime = escRampTime * 100 / globalAccelerationPercentage;

    // ESC ramp time compensation in low range
    if (lowRange)
        escRampTime = escRampTime * lowRangePercentage / 100;

    // Drive mode -------------------------------------------
    // Crawler mode for direct control -----
    crawlerMode = (masterVolume <= masterVolumeCrawlerThreshold); // Direct control, depending on master volume

    if (crawlerMode)
    { // almost no virtual inertia (just for drive train protection), for crawling competitions
        escRampTime = crawlerEscRampTime;
        brakeRampRate = map(currentThrottle, 0, 500, 1, 10);
        driveRampRate = 10;
    }
    else
    { // Virtual inertia mode -----
        // calulate throttle dependent brake & acceleration steps
        brakeRampRate = map(currentThrottle, 0, 500, 1, escBrakeSteps);
        driveRampRate = map(currentThrottle, 0, 500, 1, escAccelerationSteps);
    } // ----------------------------------------------------

    // Emergency ramp rates for falisafe
    // if (failSafe)
    // {
    //     brakeRampRate = escBrakeSteps;
    //     driveRampRate = escBrakeSteps;
    // }

    // Additional brake detection signal, applied immediately. Used to prevent sound issues, if braking very quickly
    brakeDetect = ((pulse() == 1 && tractionPulse() == -1) || (pulse() == -1 && tractionPulse() == 1));

#ifdef ESC_DEBUG
    if (millis() - lastStateTime > 300)
    { // Print the data every 300ms
        lastStateTime = millis();
        Serial.printf("ESC_DEBUG:\n");
        Serial.printf("driveState:            %i\n", driveState);
        Serial.printf("pulse():               %i\n", pulse());
        Serial.printf("escPulse():            %i\n", escPulse());
        Serial.printf("brakeDetect:           %s\n", brakeDetect ? "true" : "false");
        Serial.printf("escPulseMin:           %i\n", escPulseMin);
        Serial.printf("escPulseMinNeutral:    %i\n", escPulseMinNeutral);
        Serial.printf("escPulseMaxNeutral:    %i\n", escPulseMaxNeutral);
        Serial.printf("escPulseMax:           %i\n", escPulseMax);
        Serial.printf("brakeRampRate:         %i\n", brakeRampRate);
        Serial.printf("lowRange:              %s\n", lowRange ? "true" : "false");
        Serial.printf("currentRpm:            %i\n", currentRpm);
        Serial.printf("escPulseWidth:         %i\n", escPulseWidth);
        Serial.printf("escPulseWidthOut:      %i\n", escPulseWidthOut);
        Serial.printf("escSignal:             %i\n", escSignal);
        Serial.printf("motorDriverDuty:       %i\n", motorDriverDuty);
        Serial.printf("currentSpeed:          %i\n", currentSpeed);
        Serial.printf("speedLimit:            %i\n", speedLimit);
        Serial.printf("batteryProtection:     %s\n", batteryProtection ? "true" : "false");
        Serial.printf("batteryVoltage:        %.2f\n", batteryVoltage);
        Serial.printf("--------------------------------------\n");
    }
#endif // ESC_DEBUG

    if (millis() - escMillis > escRampTime)
    { // About very 20 - 75ms
        escMillis = millis();

        // Drive state state machine **********************************************************************************
        switch (driveState)
        {

        case 0: // Standing still ---------------------------------------------------------------------
            escIsBraking = false;
            escInReverse = false;
            escIsDriving = false;
            escPulseWidth = pulseZero3; // ESC to neutral position
#ifdef VIRTUAL_16_SPEED_SEQUENTIAL
            selectedGear = 1;
#endif

            if (pulse() == 1 && engineRunning && !neutralGear)
                driveState = 1; // Driving forward
            if (pulse() == -1 && engineRunning && !neutralGear)
                driveState = 3; // Driving backwards
            break;

        case 1: // Driving forward ---------------------------------------------------------------------
            escIsBraking = false;
            escInReverse = false;
            escIsDriving = true;
            if (escPulseWidth < tractionValue && currentSpeed < speedLimit && !batteryProtection)
            {
                if (escPulseWidth >= escPulseMaxNeutral)
                    escPulseWidth += (driveRampRate * driveRampGain); // Faster
                else
                    escPulseWidth = escPulseMaxNeutral; // Initial boost
            }
            if ((escPulseWidth > tractionValue || batteryProtection) && escPulseWidth > pulseZero3)
                escPulseWidth -= (driveRampRate * driveRampGain); // Slower

            if (gearUpShiftingPulse && shiftingAutoThrottle && !automatic && !doubleClutch)
            {                                                              // lowering RPM, if shifting up transmission
#if not defined VIRTUAL_3_SPEED && not defined VIRTUAL_16_SPEED_SEQUENTIAL // Only, if we have a real 3 speed transmission
                escPulseWidth -= currentSpeed / 4;                         // Synchronize engine speed
                                                                           // escPulseWidth -= currentSpeed * 40 / 100; // Synchronize engine speed TODO
#endif
                gearUpShiftingPulse = false;
                escPulseWidth = constrain(escPulseWidth, pulseZero3, pulseMax3);
            }
            if (gearDownShiftingPulse && shiftingAutoThrottle && !automatic && !doubleClutch)
            {                                                              // increasing RPM, if shifting down transmission
#if not defined VIRTUAL_3_SPEED && not defined VIRTUAL_16_SPEED_SEQUENTIAL // Only, if we have a real 3 speed transmission
                escPulseWidth += 50;                                       // Synchronize engine speed
                                                                           // escPulseWidth += currentSpeed;// * 40 / 100; // Synchronize engine speed TODO
#endif
                gearDownShiftingPulse = false;
                escPulseWidth = constrain(escPulseWidth, pulseZero3, pulseMax3);
            }

            if (pulse() == -1 && tractionPulse() == 1)
                driveState = 2; // Braking forward
            if (pulse() == -1 && tractionPulse() == 0)
                driveState = 3; // Driving backwards, if ESC not yet moving. Prevents state machine from hanging! v9.7.0
            if (pulse() == 0 && tractionPulse() == 0)
                driveState = 0; // standing still
            break;

        case 2: // Braking forward ---------------------------------------------------------------------
            escIsBraking = true;
            escInReverse = false;
            escIsDriving = false;
            if (escPulseWidth > pulseZero3)
                escPulseWidth -= brakeRampRate; // brake with variable deceleration
            if (escPulseWidth < pulseZero3 + brakeMargin && pulse() == -1)
                escPulseWidth = pulseZero3 + brakeMargin; // Don't go completely back to neutral, if brake applied
            if (escPulseWidth < pulseZero3 && pulse() == 0)
                escPulseWidth = pulseZero3; // Overflow prevention!

            if (pulse() == 0 && tractionPulse() == 1 && !neutralGear)
            {
                driveState = 1; // Driving forward
                airBrakeTrigger = true;
            }
            if (pulse() == 0 && tractionPulse() == 0)
            {
                driveState = 0; // standing still
                airBrakeTrigger = true;
            }
            break;

        case 3: // Driving backwards ---------------------------------------------------------------------
            escIsBraking = false;
            escInReverse = true;
            escIsDriving = true;
            if (escPulseWidth > tractionValue && currentSpeed < speedLimit && !batteryProtection)
            {
                if (escPulseWidth <= escPulseMinNeutral)
                    escPulseWidth -= (driveRampRate * driveRampGain); // Faster
                else
                    escPulseWidth = escPulseMinNeutral; // Initial boost
            }
            if ((escPulseWidth < tractionValue || batteryProtection) && escPulseWidth < pulseZero3)
                escPulseWidth += (driveRampRate * driveRampGain); // Slower

            if (gearUpShiftingPulse && shiftingAutoThrottle && !automatic && !doubleClutch)
            {                                                              // lowering RPM, if shifting up transmission
#if not defined VIRTUAL_3_SPEED && not defined VIRTUAL_16_SPEED_SEQUENTIAL // Only, if we have a real 3 speed transmission
                escPulseWidth += currentSpeed / 4;                         // Synchronize engine speed
#endif
                gearUpShiftingPulse = false;
                escPulseWidth = constrain(escPulseWidth, pulseMin3, pulseZero3);
            }
            if (gearDownShiftingPulse && shiftingAutoThrottle && !automatic && !doubleClutch)
            {                                                              // increasing RPM, if shifting down transmission
#if not defined VIRTUAL_3_SPEED && not defined VIRTUAL_16_SPEED_SEQUENTIAL // Only, if we have a real 3 speed transmission
                escPulseWidth -= 50;                                       // Synchronize engine speed
#endif
                gearDownShiftingPulse = false;
                escPulseWidth = constrain(escPulseWidth, pulseMin3, pulseZero3);
            }

            if (pulse() == 1 && tractionPulse() == -1)
                driveState = 4; // Braking backwards
            if (pulse() == 1 && tractionPulse() == 0)
                driveState = 1; // Driving forward, if ESC not yet moving. Prevents state machine from hanging! v9.7.0
            if (pulse() == 0 && tractionPulse() == 0)
                driveState = 0; // standing still
            break;

        case 4: // Braking backwards ---------------------------------------------------------------------
            escIsBraking = true;
            escInReverse = true;
            escIsDriving = false;
            if (escPulseWidth < pulseZero3)
                escPulseWidth += brakeRampRate; // brake with variable deceleration
            if (escPulseWidth > pulseZero3 - brakeMargin && pulse() == 1)
                escPulseWidth = pulseZero3 - brakeMargin; // Don't go completely back to neutral, if brake applied
            if (escPulseWidth > pulseZero3 && pulse() == 0)
                escPulseWidth = pulseZero3; // Overflow prevention!

            if (pulse() == 0 && tractionPulse() == -1 && !neutralGear)
            {
                driveState = 3; // Driving backwards
                airBrakeTrigger = true;
            }
            if (pulse() == 0 && tractionPulse() == 0)
            {
                driveState = 0; // standing still
                airBrakeTrigger = true;
            }
            break;

        } // End of state machine **********************************************************************************

        // Gain for drive ramp rate, depending on clutchEngagingPoint
        if (currentSpeed < clutchEngagingPoint)
        {
            if (!automatic && !doubleClutch)
                driveRampGain = 2; // prevent clutch from slipping too much (2)
            else
                driveRampGain = 4; // Automatic transmission needs to catch immediately (4)
        }
        else
            driveRampGain = 1;

            // ESC linearity compensation ---------------------
#ifdef QUICRUN_FUSION
        escPulseWidthOut = reMap(curveQuicrunFusion, escPulseWidth);
#elif defined QUICRUN_16BL30
        escPulseWidthOut = reMap(curveQuicrun16BL30, escPulseWidth);
#else
        escPulseWidthOut = escPulseWidth;
#endif // --------------------------------------------

        // ESC range & direction calibration -------------
#ifndef ESC_DIR
        // escSignal = escPulseWidthOut;
        escSignal = map(escPulseWidthOut, escPulseMin, escPulseMax, 1000, 2000);
#else
        escSignal = map(escPulseWidthOut, escPulseMax, escPulseMin, 1000, 2000); // direction inversed
#endif // --------------------------------------------

        // Serial.printf("escSignal = %d\n", escSignal);

        if (escSignal > 1500)
        {
            escSignal = map(escSignal, 1500, 2000, 0, 255);
            // motorTraction.motorGo(escSignal);
        }
        else if (escSignal < 1500)
        {
            escSignal = map(escSignal, 1500, 1000, 0, 255);
            // motorA.motorRev(escSignal);
        }
        else
        {
            // motorTraction.motorBrake();
            // motorTraction.motorStop();
        }

        // Serial.printf("escSignal2 = %d\n", escSignal);

        // Calculate a speed value from the pulsewidth signal (used as base for engine sound RPM while clutch is engaged)
        if (escPulseWidth > pulseMaxNeutral3)
        {
            currentSpeed = map(escPulseWidth, pulseMaxNeutral3, pulseMax3, 0, 500);
        }
        else if (escPulseWidth < pulseMinNeutral3)
        {
            currentSpeed = map(escPulseWidth, pulseMinNeutral3, pulseMin3, 0, 500);
        }
        else
            currentSpeed = 0;
    }
#endif
}

void setupTraction()
{
    //
}

#endif // __TRACTION_H__