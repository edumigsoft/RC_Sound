#ifndef __PLAYBACK_H__
#define __PLAYBACK_H__

#include <Arduino.h>
#include "variables.h"

void dacOffsetFade()
{
    if (!dacInit)
    {
        if (micros() - dacOffsetMicros > 100)
        { // Every 0.1ms
            dacOffsetMicros = micros();
            dacOffset++; // fade DAC offset slowly to prevent it from popping, if ESP32 powered up after amplifier
            if (dacOffset == 128)
                dacInit = true;
        }
    }
}

void IRAM_ATTR variablePlaybackTimer()
{
    static uint32_t attenuatorMillis = 0;
    static uint32_t curEngineSample = 0;          // Index of currently loaded engine sample
    static uint32_t curRevSample = 0;             // Index of currently loaded engine rev sample
    static uint32_t curTurboSample = 0;           // Index of currently loaded turbo sample
    static uint32_t curFanSample = 0;             // Index of currently loaded fan sample
    static uint32_t curChargerSample = 0;         // Index of currently loaded charger sample
    static uint32_t curStartSample = 0;           // Index of currently loaded start sample
    static uint32_t curJakeBrakeSample = 0;       // Index of currently loaded jake brake sample
    static uint32_t lastDieselKnockSample = 0;    // Index of last Diesel knock sample
    static uint16_t attenuator = 0;               // Used for volume adjustment during engine switch off
    static uint16_t speedPercentage = 0;          // slows the engine down during shutdown
    static int32_t a, a1, a2, a3, b, c, d, e = 0; // Input signals for mixer: a = engine, b = additional sound, c = turbo sound, d = fan sound, e = supercharger sound
    static int32_t f = 0;                         // Input signals for mixer: f = hydraulic pump
    static int32_t g = 0;                         // Input signals for mixer: g = train track rattle
    uint8_t a1Multi = 0;                          // Volume multipliers

    switch (engineState)
    {

    case OFF:                                                     // Engine off -----------------------------------------------------------------------
        variableTimerTicks = 4000000 / startSampleRate;           // our fixed sampling rate
        timerAlarmWrite(variableTimer, variableTimerTicks, true); // // change timer ticks, autoreload true

        a = 0; // volume = zero
        if (engineOn)
        {
            engineState = STARTING;
            engineStart = true;
        }
        break;

    case STARTING:                                                // Engine start --------------------------------------------------------------------
        variableTimerTicks = 4000000 / startSampleRate;           // our fixed sampling rate
        timerAlarmWrite(variableTimer, variableTimerTicks, true); // // change timer ticks, autoreload true

        if (curStartSample < startSampleCount - 1)
        {
            curStartSample++;
        }
        else
        {
            curStartSample = 0;
            engineState = RUNNING;
            engineStart = false;
            engineRunning = true;
            airBrakeTrigger = true;
        }
        break;

    case RUNNING: // Engine running ------------------------------------------------------------------

        // Engine idle & revving sounds (mixed together according to engine rpm, new in v5.0)
        variableTimerTicks = engineSampleRate;                    // our variable idle sampling rate!
        timerAlarmWrite(variableTimer, variableTimerTicks, true); // // change timer ticks, autoreload true

        if (!engineJakeBraking && !blowoffTrigger)
        {
            if (curEngineSample < sampleCount - 1)
            {
                a1 = (samples[curEngineSample] * throttleDependentVolume / 100 * idleVolumePercentage / 100); // Idle sound
                a3 = 0;
                curEngineSample++;

                // Optional rev sound, recorded at medium rpm. Note, that it needs to represent the same number of ignition cycles as the
                // idle sound. For example 4 or 8 for a V8 engine. It also needs to have about the same length. In order to adjust the length
                // or "revSampleCount", change the "Rate" setting in Audacity until it is about the same.
#ifdef REV_SOUND
                a2 = (revSamples[curRevSample] * throttleDependentRevVolume / 100 * revVolumePercentage / 100); // Rev sound
                if (curRevSample < revSampleCount)
                    curRevSample++;
#endif

                // Trigger throttle dependent Diesel ignition "knock" sound (played in the fixed sample rate interrupt)
                if (curEngineSample - lastDieselKnockSample > (sampleCount / dieselKnockInterval))
                {
                    dieselKnockTrigger = true;
                    dieselKnockTriggerFirst = false;
                    lastDieselKnockSample = curEngineSample;
                }
            }
            else
            {
                curEngineSample = 0;
                if (jakeBrakeRequest)
                    engineJakeBraking = true;
#ifdef REV_SOUND
                curRevSample = 0;
#endif
                lastDieselKnockSample = 0;
                dieselKnockTrigger = true;
                dieselKnockTriggerFirst = true;
            }
            curJakeBrakeSample = 0;
        }
        else
        { // Jake brake sound ----
#ifdef JAKE_BRAKE_SOUND
            a3 = (jakeBrakeSamples[curJakeBrakeSample] * rpmDependentJakeBrakeVolume / 100 * jakeBrakeVolumePercentage / 100); // Jake brake sound
            a2 = 0;
            a1 = 0;
            if (curJakeBrakeSample < jakeBrakeSampleCount - 1)
                curJakeBrakeSample++;
            else
            {
                curJakeBrakeSample = 0;
                if (!jakeBrakeRequest)
                    engineJakeBraking = false;
            }

            curEngineSample = 0;
            curRevSample = 0;
#endif
        }

        // Engine sound mixer ----
#ifdef REV_SOUND
        // Mixing the idle and rev sounds together, according to engine rpm
        // Below the "revSwitchPoint" target, the idle volume precentage is 90%, then falling to 0% @ max. rpm.
        // The total of idle and rev volume percentage is always 100%

        if (currentRpm > revSwitchPoint)
            a1Multi = map(currentRpm, idleEndPoint, revSwitchPoint, 0, idleVolumeProportionPercentage);
        else
            a1Multi = idleVolumeProportionPercentage; // 90 - 100% proportion
        if (currentRpm > idleEndPoint)
            a1Multi = 0;

        a1 = a1 * a1Multi / 100;         // Idle volume
        a2 = a2 * (100 - a1Multi) / 100; // Rev volume

        a = a1 + a2 + a3; // Idle and rev sounds mixed together
#else
        a = a1 + a3; // Idle sound only
#endif

        // Turbo sound ----------------------------------
        if (curTurboSample < turboSampleCount - 1)
        {
            c = (turboSamples[curTurboSample] * throttleDependentTurboVolume / 100 * turboVolumePercentage / 100);
            curTurboSample++;
        }
        else
        {
            curTurboSample = 0;
        }

        // Fan sound -----------------------------------
        if (curFanSample < fanSampleCount - 1)
        {
            d = (fanSamples[curFanSample] * throttleDependentFanVolume / 100 * fanVolumePercentage / 100);
            curFanSample++;
        }
        else
        {
            curFanSample = 0;
        }
#if defined GEARBOX_WHINING
        if (neutralGear)
            d = 0; // used for gearbox whining simulation, so not active in gearbox neutral
#endif

        // Supercharger sound --------------------------
        if (curChargerSample < chargerSampleCount - 1)
        {
            e = (chargerSamples[curChargerSample] * throttleDependentChargerVolume / 100 * chargerVolumePercentage / 100);
            curChargerSample++;
        }
        else
        {
            curChargerSample = 0;
        }

        if (!engineOn)
        {
            speedPercentage = 100;
            attenuator = 1;
            engineState = STOPPING;
            engineStop = true;
            engineRunning = false;
        }
        break;

    case STOPPING:                                                         // Engine stop --------------------------------------------------------------------
        variableTimerTicks = 4000000 / sampleRate * speedPercentage / 100; // our fixed sampling rate
        timerAlarmWrite(variableTimer, variableTimerTicks, true);          // // change timer ticks, autoreload true

        if (curEngineSample < sampleCount - 1)
        {
            a = (samples[curEngineSample] * throttleDependentVolume / 100 * idleVolumePercentage / 100 / attenuator);
            curEngineSample++;
        }
        else
        {
            curEngineSample = 0;
        }

        // fade engine sound out
        if (millis() - attenuatorMillis > 100)
        { // Every 50ms
            attenuatorMillis = millis();
            attenuator++;          // attenuate volume
            speedPercentage += 20; // make it slower (10)
        }

        if (attenuator >= 50 || speedPercentage >= 500)
        { // 50 & 500
            a = 0;
            speedPercentage = 100;
            parkingBrakeTrigger = true;
            engineState = PARKING_BRAKE;
            engineStop = false;
        }
        break;

    case PARKING_BRAKE: // parking brake bleeding air sound after engine is off ----------------------------

        if (!parkingBrakeTrigger)
        {
            engineState = OFF;
        }
        break;

    } // end of switch case

    // DAC output (groups a, b, c mixed together) ************************************************************************
    dacWrite(DAC1, constrain(((a * 8 / 10) + (b / 2) + (c / 5) + (d / 5) + (e / 5) + f + g) * masterVolume / 100 + dacOffset, 0, 255)); // Mix signals, add 128 offset, write  to DAC
}

void IRAM_ATTR fixedPlaybackTimer()
{
    static uint32_t curHornSample = 0;                            // Index of currently loaded horn sample
    static uint32_t curSirenSample = 0;                           // Index of currently loaded siren sample
    static uint32_t curSound1Sample = 0;                          // Index of currently loaded sound1 sample
    static uint32_t curReversingSample = 0;                       // Index of currently loaded reversing beep sample
    static uint32_t curIndicatorSample = 0;                       // Index of currently loaded indicator tick sample
    static uint32_t curWastegateSample = 0;                       // Index of currently loaded wastegate sample
    static uint32_t curBrakeSample = 0;                           // Index of currently loaded brake sound sample
    static uint32_t curParkingBrakeSample = 0;                    // Index of currently loaded brake sound sample
    static uint32_t curShiftingSample = 0;                        // Index of currently loaded shifting sample
    static uint32_t curDieselKnockSample = 0;                     // Index of currently loaded Diesel knock sample
    static int32_t a, a1, a2 = 0;                                 // Input signals "a" for mixer
    static int32_t b, b0, b1, b2, b3, b4, b5, b6, b7, b8, b9 = 0; // Input signals "b" for mixer
    static int32_t c, c1, c2, c3 = 0;                             // Input signals "c" for mixer
    static int32_t d, d1, d2 = 0;                                 // Input signals "d" for mixer
    static boolean knockSilent = 0;                               // This knock will be more silent
    static boolean knockMedium = 0;                               // This knock will be medium
    static uint8_t curKnockCylinder = 0;                          // Index of currently ignited zylinder

    // Group "a" (horn & siren) ******************************************************************

    if (hornTrigger || hornLatch)
    {
        fixedTimerTicks = 4000000 / hornSampleRate;         // our fixed sampling rate
        timerAlarmWrite(fixedTimer, fixedTimerTicks, true); // // change timer ticks, autoreload true

        if (curHornSample < hornSampleCount - 1)
        {
            a1 = (hornSamples[curHornSample] * hornVolumePercentage / 100);
            curHornSample++;
#ifdef HORN_LOOP // Optional "endless loop" (points to be defined manually in horn file)
            if (hornTrigger && curHornSample == hornLoopEnd)
                curHornSample = hornLoopBegin; // Loop, if trigger still present
#endif
        }
        else
        { // End of sample
            curHornSample = 0;
            a1 = 0;
            hornLatch = false;
        }
    }

    if (sirenTrigger || sirenLatch)
    {
        fixedTimerTicks = 4000000 / sirenSampleRate;        // our fixed sampling rate
        timerAlarmWrite(fixedTimer, fixedTimerTicks, true); // // change timer ticks, autoreload true

#if defined SIREN_STOP
        if (!sirenTrigger)
        {
            sirenLatch = false;
            curSirenSample = 0;
            a2 = 0;
        }
#endif

        if (curSirenSample < sirenSampleCount - 1)
        {
            a2 = (sirenSamples[curSirenSample] * sirenVolumePercentage / 100);
            curSirenSample++;
#ifdef SIREN_LOOP // Optional "endless loop" (points to be defined manually in siren file)
            if (sirenTrigger && curSirenSample == sirenLoopEnd)
                curSirenSample = sirenLoopBegin; // Loop, if trigger still present
#endif
        }
        else
        { // End of sample
            curSirenSample = 0;
            a2 = 0;
            sirenLatch = false;
        }
    }
    if (curSirenSample > 10 && curSirenSample < 500)
        cannonFlash = true; // Tank cannon flash triggering in TRACKED_MODE
    else
        cannonFlash = false;

    // Group "b" (other sounds) **********************************************************************

    // Sound 1 "b0" ----
    if (sound1trigger)
    {
        fixedTimerTicks = 4000000 / sound1SampleRate;       // our fixed sampling rate
        timerAlarmWrite(fixedTimer, fixedTimerTicks, true); // // change timer ticks, autoreload true

        if (curSound1Sample < sound1SampleCount - 1)
        {
            b0 = (sound1Samples[curSound1Sample] * sound1VolumePercentage / 100);
            curSound1Sample++;
        }
        else
        {
            sound1trigger = false;
        }
    }
    else
    {
        curSound1Sample = 0; // ensure, next sound will start @ first sample
        b0 = 0;
    }

    // Reversing beep sound "b1" ----
    if (engineRunning && escInReverse)
    {
        fixedTimerTicks = 4000000 / reversingSampleRate;    // our fixed sampling rate
        timerAlarmWrite(fixedTimer, fixedTimerTicks, true); // // change timer ticks, autoreload true

        if (curReversingSample < reversingSampleCount - 1)
        {
            b1 = (reversingSamples[curReversingSample] * reversingVolumePercentage / 100);
            curReversingSample++;
        }
        else
        {
            curReversingSample = 0;
        }
    }
    else
    {
        curReversingSample = 0; // ensure, next sound will start @ first sample
        b1 = 0;
    }

    // Indicator tick sound "b2" ----------------------------------------------------------------------
#if not defined NO_INDICATOR_SOUND
    if (indicatorSoundOn)
    {
        fixedTimerTicks = 4000000 / indicatorSampleRate;    // our fixed sampling rate
        timerAlarmWrite(fixedTimer, fixedTimerTicks, true); // // change timer ticks, autoreload true

        if (curIndicatorSample < indicatorSampleCount - 1)
        {
            b2 = (indicatorSamples[curIndicatorSample] * indicatorVolumePercentage / 100);
            curIndicatorSample++;
        }
        else
        {
            indicatorSoundOn = false;
        }
    }
    else
    {
        curIndicatorSample = 0; // ensure, next sound will start @ first sample
        b2 = 0;
    }
#endif

    // Wastegate (blowoff) sound, triggered after rapid throttle drop -----------------------------------
    if (wastegateTrigger)
    {
        if (curWastegateSample < wastegateSampleCount - 1)
        {
            b3 = (wastegateSamples[curWastegateSample] * rpmDependentWastegateVolume / 100 * wastegateVolumePercentage / 100);
            curWastegateSample++;
        }
        else
        {
            wastegateTrigger = false;
        }
    }
    else
    {
        b3 = 0;
        curWastegateSample = 0; // ensure, next sound will start @ first sample
    }

    // Air brake release sound, triggered after stop -----------------------------------------------
    if (airBrakeTrigger)
    {
        if (curBrakeSample < brakeSampleCount - 1)
        {
            b4 = (brakeSamples[curBrakeSample] * brakeVolumePercentage / 100);
            curBrakeSample++;
        }
        else
        {
            airBrakeTrigger = false;
        }
    }
    else
    {
        b4 = 0;
        curBrakeSample = 0; // ensure, next sound will start @ first sample
    }

    // Air parking brake attaching sound, triggered after engine off --------------------------------
    if (parkingBrakeTrigger)
    {
        if (curParkingBrakeSample < parkingBrakeSampleCount - 1)
        {
            b5 = (parkingBrakeSamples[curParkingBrakeSample] * parkingBrakeVolumePercentage / 100);
            curParkingBrakeSample++;
        }
        else
        {
            parkingBrakeTrigger = false;
        }
    }
    else
    {
        b5 = 0;
        curParkingBrakeSample = 0; // ensure, next sound will start @ first sample
    }

    // Pneumatic gear shifting sound, triggered while shifting the TAMIYA 3 speed transmission ------
    if (shiftingTrigger && engineRunning && !automatic && !doubleClutch)
    {
        if (curShiftingSample < shiftingSampleCount - 1)
        {
            b6 = (shiftingSamples[curShiftingSample] * shiftingVolumePercentage / 100);
            curShiftingSample++;
        }
        else
        {
            shiftingTrigger = false;
        }
    }
    else
    {
        b6 = 0;
        curShiftingSample = 0; // ensure, next sound will start @ first sample
    }

    // Diesel ignition "knock" is played in fixed sample rate section, because we don't want changing pitch! ------
    if (dieselKnockTriggerFirst)
    {
        dieselKnockTriggerFirst = false;
        curKnockCylinder = 0;
    }

    if (dieselKnockTrigger)
    {
        dieselKnockTrigger = false;
        curKnockCylinder++; // Count ignition sequence
        curDieselKnockSample = 0;
    }

#ifdef V8 // (former ADAPTIVE_KNOCK_VOLUME, rename it in your config file!)
    // Ford or Scania V8 ignition sequence: 1 - 5 - 4 - 2* - 6 - 3 - 7 - 8* (* = louder knock pulses, because 2nd exhaust in same manifold after 90°)
    if (curKnockCylinder == 4 || curKnockCylinder == 8)
        knockSilent = false;
    else
        knockSilent = true;
#endif

#ifdef V8_MEDIUM // (former ADAPTIVE_KNOCK_VOLUME, rename it in your config file!)
    // This is EXPERIMENTAL!! TODO
    if (curKnockCylinder == 5 || curKnockCylinder == 1)
        knockMedium = false;
    else
        knockMedium = true;
#endif

#ifdef V8_468 // (Chevy 468, containing 16 ignition pulses)
    // 1th, 5th, 9th and 13th are the loudest
    // Ignition sequence: 1 - 8 - 4* - 3 - 6 - 5 - 7* - 2
    if (curKnockCylinder == 1 || curKnockCylinder == 5 || curKnockCylinder == 9 || curKnockCylinder == 13)
        knockSilent = false;
    else
        knockSilent = true;
#endif

#ifdef V2
    // V2 engine: 1st and 2nd knock pulses (of 4) will be louder
    if (curKnockCylinder == 1 || curKnockCylinder == 2)
        knockSilent = false;
    else
        knockSilent = true;
#endif

#ifdef R6
    // R6 inline 6 engine: 6th knock pulse (of 6) will be louder
    if (curKnockCylinder == 6)
        knockSilent = false;
    else
        knockSilent = true;
#endif

#ifdef R6_2
    // R6 inline 6 engine: 6th and 3rd knock pulse (of 6) will be louder
    if (curKnockCylinder == 6 || curKnockCylinder == 3)
        knockSilent = false;
    else
        knockSilent = true;
#endif

    if (curDieselKnockSample < knockSampleCount)
    {
#if defined RPM_DEPENDENT_KNOCK // knock volume also depending on engine rpm
        b7 = (knockSamples[curDieselKnockSample] * dieselKnockVolumePercentage / 100 * throttleDependentKnockVolume / 100 * rpmDependentKnockVolume / 100);
#else                        // Just depending on throttle
        b7 = (knockSamples[curDieselKnockSample] * dieselKnockVolumePercentage / 100 * throttleDependentKnockVolume / 100);
#endif
        curDieselKnockSample++;
        if (knockSilent && !knockMedium)
            b7 = b7 * dieselKnockAdaptiveVolumePercentage / 100; // changing knock volume according to engine type and cylinder!
        if (knockMedium)
            b7 = b7 * dieselKnockAdaptiveVolumePercentage / 75;
    }

    // Group "d" (additional sounds) **********************************************************************

#if defined TIRE_SQUEAL
    // Tire squeal sound -----------------------
    if (curTireSquealSample < tireSquealSampleCount - 1)
    {
        d1 = (tireSquealSamples[curTireSquealSample] * tireSquealVolumePercentage / 100 * tireSquealVolume / 100);
        curTireSquealSample++;
    }
    else
    {
        d1 = 0;
        curTireSquealSample = 0;
    }
#endif

#if defined BATTERY_PROTECTION
    // Out of fuel sound, triggered by battery voltage -----------------------------------------------
    if (outOfFuelMessageTrigger)
    {
        if (curOutOfFuelSample < outOfFuelSampleCount - 1)
        {
            d2 = (outOfFuelSamples[curOutOfFuelSample] * outOfFuelVolumePercentage / 100);
            curOutOfFuelSample++;
        }
        else
        {
            outOfFuelMessageTrigger = false;
        }
    }
    else
    {
        d2 = 0;
        curOutOfFuelSample = 0; // ensure, next sound will start @ first sample
    }
#endif

    // Mixing sounds together **********************************************************************
    a = a1 + a2; // Horn & siren
    // if (a < 2 && a > -2) a = 0; // Remove noise floor TODO, experimental
    b = b0 * 5 + b1 + b2 / 2 + b3 + b4 + b5 + b6 + b7 + b8 + b9; // Other sounds
    c = c1 + c2 + c3;                                            // Excavator sounds
    d = d1 + d2;                                                 // Additional sounds

    // DAC output (groups mixed together) ****************************************************************************
    dacWrite(DAC2, constrain(((a * 8 / 10) + (b * 2 / 10) + c + d) * masterVolume / 100 + dacOffset, 0, 255)); // Mix signals, add 128 offset, write result to DAC
}

#endif // __PLAYBACK_H__