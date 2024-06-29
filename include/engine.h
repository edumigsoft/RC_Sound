#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "variables.h"
#include "curves.h"
#include "4_Transmission.h"
#include "5_Shaker.h"

void engineOnOff()
{
    //   static unsigned long idleDelayMillis;

    //   // Engine automatically switched on or off depending on throttle position and 15s delay timne
    //   if (currentThrottle > 80 || driveState != 0)
    //     idleDelayMillis = millis(); // reset delay timer, if throttle not in neutral

    // #ifdef AUTO_ENGINE_ON_OFF
    //   if (millis() - idleDelayMillis > 15000)
    //   {
    //     engineOn = false; // after delay, switch engine off
    //   }
    // #endif

    // #ifdef AUTO_LIGHTS
    //   if (millis() - idleDelayMillis > 10000)
    //   {
    //     lightsOn = false; // after delay, switch light off
    //   }
    // #endif

    // Engine start detection
    if (currentThrottle > 100 && !airBrakeTrigger)
    {
        engineOn = true;

#ifdef AUTO_LIGHTS
        lightsOn = true;
#endif
    }
}

void engineMassSimulation()
{
    static int32_t targetRpm = 0;   // The engine RPM target
    static int32_t _currentRpm = 0; // Private current RPM (to prevent conflict with core 1)
    static int32_t _currentThrottle = 0;
    static int32_t lastThrottle;
    uint16_t converterSlip;
    static unsigned long throtMillis;
    static unsigned long wastegateMillis;
    // static unsigned long blowoffMillis;
    uint8_t timeBase;

#ifdef SUPER_SLOW
    timeBase = 6; // super slow running, heavy engines, for example locomotive diesels
#else
    timeBase = 2;
#endif

    _currentThrottle = currentThrottle;

    if (millis() - throtMillis > timeBase)
    { // Every 2 or 6ms
        throtMillis = millis();

        if (_currentThrottle > 500)
            _currentThrottle = 500;

        // Virtual clutch **********************************************************************************
        // Normal mode ---
        // if ((currentSpeed < clutchEngagingPoint && _currentRpm < maxClutchSlippingRpm) || gearUpShiftingInProgress || gearDownShiftingInProgress || neutralGear || _currentRpm < 200) { // TODO Bug?
        if ((currentSpeed < clutchEngagingPoint && _currentRpm < maxClutchSlippingRpm) || gearUpShiftingInProgress || gearDownShiftingInProgress || neutralGear)
        {
            clutchDisengaged = true;
        }
        else
        {
            clutchDisengaged = false;
        }

        // Transmissions ***********************************************************************************

        // automatic transmission ----
        if (automatic)
        {
            // Torque converter slip calculation
            if (selectedAutomaticGear < 2)
                converterSlip = engineLoad * torqueconverterSlipPercentage / 100 * 2; // more slip in first and reverse gear
            else
                converterSlip = engineLoad * torqueconverterSlipPercentage / 100;

            if (!neutralGear)
                targetRpm = currentSpeed * gearRatio[selectedAutomaticGear] / 10 + converterSlip; // Compute engine RPM
            else
                targetRpm = reMap(curveLinear, _currentThrottle);
        }
        else if (doubleClutch)
        {
            // double clutch transmission
            if (!neutralGear)
                targetRpm = currentSpeed * gearRatio[selectedAutomaticGear] / 10; // Compute engine RPM
            else
                targetRpm = reMap(curveLinear, _currentThrottle);
        }
        else
        {
            // Manual transmission ----
            if (clutchDisengaged)
            { // Clutch disengaged: Engine revving allowed
#if defined VIRTUAL_16_SPEED_SEQUENTIAL
                targetRpm = _currentThrottle;
#else
                targetRpm = reMap(curveLinear, _currentThrottle);

#endif
            }
            else
            { // Clutch engaged: Engine rpm synchronized with ESC power (speed)

#if defined VIRTUAL_3_SPEED || defined VIRTUAL_16_SPEED_SEQUENTIAL                                          // Virtual 3 speed or sequential 16 speed transmission
                targetRpm = reMap(curveLinear, (currentSpeed * virtualManualGearRatio[selectedGear] / 10)); // Add virtual gear ratios
                if (targetRpm > 500)
                    targetRpm = 500;

#else // Real 3 speed transmission
                targetRpm = reMap(curveLinear, currentSpeed);
#endif
            }
        }

        // Engine RPM **************************************************************************************

        if (escIsBraking && currentSpeed < clutchEngagingPoint)
            targetRpm = 0; // keep engine @idle rpm, if braking at very low speed
        if (targetRpm > 500)
            targetRpm = 500;

        // Accelerate engine
        if (targetRpm > (_currentRpm + acc) && (_currentRpm + acc) < maxRpm && engineState == RUNNING && engineRunning)
        {
            if (!airBrakeTrigger)
            { // No acceleration, if brake release noise still playing
                if (!gearDownShiftingInProgress)
                    _currentRpm += acc;
                else
                    _currentRpm += acc / 2; // less aggressive rpm rise while downshifting
                if (_currentRpm > maxRpm)
                    _currentRpm = maxRpm;
            }
        }

        // Decelerate engine
        if (targetRpm < _currentRpm)
        {
            _currentRpm -= dec;
            if (_currentRpm < minRpm)
                _currentRpm = minRpm;
        }

#if (defined VIRTUAL_3_SPEED || defined VIRTUAL_16_SPEED_SEQUENTIAL) and not defined STEAM_LOCOMOTIVE_MODE
        // Limit top speed, depending on manual gear ratio. Ensures, that the engine will not blow up!
        if (!automatic && !doubleClutch)
            speedLimit = maxRpm * 10 / virtualManualGearRatio[selectedGear];
#endif

        // Speed (sample rate) output
        engineSampleRate = map(_currentRpm, minRpm, maxRpm, maxSampleInterval, minSampleInterval); // Idle

        // if ( xSemaphoreTake( xRpmSemaphore, portMAX_DELAY ) )
        //{
        currentRpm = _currentRpm;
        // xSemaphoreGive( xRpmSemaphore ); // Now free or "Give" the semaphore for others.
        // }
    }

    // Prevent Wastegate from being triggered while downshifting
    if (gearDownShiftingInProgress)
        wastegateMillis = millis();

    // Trigger Wastegate, if throttle rapidly dropped
    if (lastThrottle - _currentThrottle > 70 && !escIsBraking && millis() - wastegateMillis > 1000)
    {
        wastegateMillis = millis();
        wastegateTrigger = true;
    }

#if defined JAKEBRAKE_ENGINE_SLOWDOWN && defined JAKE_BRAKE_SOUND
    // Use jake brake to slow down engine while releasing throttle in neutral or during upshifting while applying throttle
    // for some vehicles like Volvo FH open pipe. See example: https://www.youtube.com/watch?v=MU1iwzl33Zw&list=LL&index=4
    if (!wastegateTrigger)
        blowoffMillis = millis();
    blowoffTrigger = ((gearUpShiftingInProgress || neutralGear) && millis() - blowoffMillis > 20 && millis() - blowoffMillis < 250);
#endif

    lastThrottle = _currentThrottle;
}

void automaticGearSelector()
{
    static unsigned long gearSelectorMillis;
    static unsigned long lastUpShiftingMillis;
    static unsigned long lastDownShiftingMillis;
    uint16_t downShiftPoint = 200;
    uint16_t upShiftPoint = 490;
    static int32_t _currentRpm = 0; // Private current RPM (to prevent conflict with core 1)

    // if ( xSemaphoreTake( xRpmSemaphore, portMAX_DELAY ) )
    //{
    _currentRpm = currentRpm;
    // xSemaphoreGive( xRpmSemaphore ); // Now free or "Give" the semaphore for others.
    // }

    if (millis() - gearSelectorMillis > 100)
    { // Waiting for 100ms is very important. Otherwise gears are skipped!
        gearSelectorMillis = millis();

        // compute load dependent shift points (less throttle = less rpm before shifting up, kick down will shift back!)
        upShiftPoint = map(engineLoad, 0, 180, 390, 490);   // 390, 490
        downShiftPoint = map(engineLoad, 0, 180, 150, 250); // 150, 250

        if (escInReverse)
        { // Reverse (only one gear)
            selectedAutomaticGear = 0;
        }
        else
        { // Forward (multiple gears)

            // Adaptive shift points
            if (millis() - lastDownShiftingMillis > 500 && _currentRpm >= upShiftPoint && engineLoad < 5)
            {                            // 500ms locking timer!
                selectedAutomaticGear++; // Upshifting (load maximum is important to prevent gears from oscillating!)
                lastUpShiftingMillis = millis();
            }
            if (millis() - lastUpShiftingMillis > 600 && selectedAutomaticGear > 1 && (_currentRpm <= downShiftPoint || engineLoad > 100))
            {                            // 600ms locking timer! TODO was 1000
                selectedAutomaticGear--; // Downshifting incl. kickdown
                lastDownShiftingMillis = millis();
            }

            selectedAutomaticGear = constrain(selectedAutomaticGear, 1, NumberOfAutomaticGears);
        }

#ifdef AUTO_TRANS_DEBUG
        Serial.printf("AUTO_TRANS_DEBUG:\n");
        Serial.printf("currentThrottle: %i\n", currentThrottle);
        Serial.printf("selectedAutomaticGear: %i\n", selectedAutomaticGear);
        Serial.printf("engineLoad: %i\n", engineLoad);
        Serial.printf("upShiftPoint: %i\n", upShiftPoint);
        Serial.printf("_currentRpm: %i\n", _currentRpm);
        Serial.printf("downShiftPoint: %i\n", downShiftPoint);
        Serial.printf("-----------------------------------\n");
#endif
    }
}

void shaker()
{
    int32_t shakerRpm = 0;

    // Set desired shaker rpm
    if (engineRunning)
        shakerRpm = map(currentRpm, minRpm, maxRpm, shakerIdle, shakerFullThrottle);
    if (engineStart)
        shakerRpm = shakerStart;
    if (engineStop)
        shakerRpm = shakerStop;

    // Shaker on / off
    if (engineRunning || engineStart || engineStop)
        shakerMotor.pwm(shakerRpm);
    else
        shakerMotor.off();
}

void gearboxDetection()
{
    static uint8_t previousGear = 1;
    static bool previousReverse;
    // static bool sequentialLock;
    // static bool overdrive = false;
    static unsigned long upShiftingMillis;
    static unsigned long downShiftingMillis;
    static unsigned long lastShiftingMillis; // This timer is used to prevent transmission from oscillating!

#if defined TRACKED_MODE or defined STEAM_LOCOMOTIVE_MODE // CH2 is used for left throttle in TRACKED_MODE --------------------------------
    selectedGear = 2;

#else // only active, if not in TRACKED_MODE -------------------------------------------------------------

#if defined OVERDRIVE && defined VIRTUAL_3_SPEED // Additional 4th gear mode for virtual 3 speed ********************************
    if (!crawlerMode)
    {
        // The 4th gear (overdrive) is engaged automatically, if driving @ full throttle in 3rd gear
        if (currentRpm > 490 && selectedGear == 3 && engineLoad < 5 && currentThrottle > 490 && millis() - lastShiftingMillis > 2000)
        {
            overdrive = true;
        }
        if (!escIsBraking)
        { // Lower downshift point, if not braking
            if (currentRpm < 200 && millis() - lastShiftingMillis > 2000)
            {
                overdrive = false;
            }
        }
        else
        { // Higher downshift point, if braking
            if ((currentRpm < 400 || engineLoad > 150) && millis() - lastShiftingMillis > 2000)
            {
                overdrive = false;
            }
        }
        if (selectedGear < 3)
            overdrive = false;
    }
#endif                                           // End of overdrive ******************************************************************************************************

#if not defined VIRTUAL_16_SPEED_SEQUENTIAL && not defined SEMI_AUTOMATIC // 3 gears, selected by 3 position switch **************
    if (overdrive && selectedGear == 3)
        selectedGear = 4;
#endif                                                                    // End of manual 3 speed *************************************************************************************************

#if defined VIRTUAL_16_SPEED_SEQUENTIAL // 16 gears, selected by up / down impulses *********************************************
    if (pulseWidth[2] > 1700 && selectedGear < 16 && !sequentialLock)
    {
        sequentialLock = true;
        selectedGear++;
    }
    else if (pulseWidth[2] < 1300 && selectedGear > 1 && !sequentialLock)
    {
        sequentialLock = true;
        selectedGear--;
    }
    if (pulseWidth[2] > 1400 && pulseWidth[2] < 1600)
        sequentialLock = false;
#endif                                  // End of VIRTUAL_16_SPEED_SEQUENTIAL *************************************************************************************

#if defined SEMI_AUTOMATIC // gears not controlled by the 3 position switch but by RPM limits ************************************
    if (currentRpm > 490 && selectedGear < 3 && engineLoad < 5 && currentThrottle > 490 && millis() - lastShiftingMillis > 2000)
    {
        selectedGear++;
    }
    if (!escIsBraking)
    { // Lower downshift point, if not braking
        if (currentRpm < 200 && selectedGear > 1 && millis() - lastShiftingMillis > 2000)
        {
            selectedGear--; //
        }
    }
    else
    { // Higher downshift point, if braking
        if ((currentRpm < 400 || engineLoad > 150) && selectedGear > 1 && millis() - lastShiftingMillis > 2000)
        {
            selectedGear--; // Higher downshift point, if braking
        }
    }
    if (neutralGear || escInReverse)
        selectedGear = 1;
#endif                     // End of SEMI_AUTOMATIC **************************************************************************************************

    // Gear upshifting detection
    if (selectedGear > previousGear)
    {
        gearUpShiftingInProgress = true;
        gearUpShiftingPulse = true;
        shiftingTrigger = true;
        previousGear = selectedGear;
        lastShiftingMillis = millis();
    }

    // Gear upshifting duration
    static uint16_t upshiftingDuration = 700;
    if (!gearUpShiftingInProgress)
        upShiftingMillis = millis();
    if (millis() - upShiftingMillis > upshiftingDuration)
    {
        gearUpShiftingInProgress = false;
    }
    // Double-clutch (Zwischengas während dem Hochschalten)
#if defined DOUBLE_CLUTCH
    upshiftingDuration = 900;
    doubleClutchInProgress = (millis() - upShiftingMillis >= 500 && millis() - upShiftingMillis < 600); // Apply full throttle
#endif

    // Gear downshifting detection
    if (selectedGear < previousGear)
    {
        gearDownShiftingInProgress = true;
        gearDownShiftingPulse = true;
        shiftingTrigger = true;
        previousGear = selectedGear;
        lastShiftingMillis = millis();
    }

    // Gear downshifting duration
    if (!gearDownShiftingInProgress)
        downShiftingMillis = millis();
    if (millis() - downShiftingMillis > 300)
    {
        gearDownShiftingInProgress = false;
    }

    // Reverse gear engaging / disengaging detection
    if (escInReverse != previousReverse)
    {
        previousReverse = escInReverse;
        shiftingTrigger = true; // Play shifting sound
    }

#ifdef MANUAL_TRANS_DEBUG
    static unsigned long manualTransDebugMillis;
    if (millis() - manualTransDebugMillis > 100)
    {
        manualTransDebugMillis = millis();
        Serial.printf("MANUAL_TRANS_DEBUG:\n");
        Serial.printf("currentThrottle: %i\n", currentThrottle);
        Serial.printf("selectedGear: %i\n", selectedGear);
        Serial.printf("overdrive: %i\n", overdrive);
        Serial.printf("engineLoad: %i\n", engineLoad);
        Serial.printf("sequentialLock: %s\n", sequentialLock ? "true" : "false");
        Serial.printf("currentRpm: %i\n", currentRpm);
        Serial.printf("currentSpeed: %i\n", currentSpeed);
        Serial.printf("---------------------------------\n");
    }
#endif // MANUAL_TRANS_DEBUG

#endif // End of not TRACKED_MODE -----------------------------------------------------------------------
}

void mapThrottle()
{
    // Input is around 1000 - 2000us, output 0-500 for forward and backwards

#if defined TRACKED_MODE          // Dual throttle input for caterpillar vehicles ------------------
    int16_t currentThrottleLR[4]; // 2 & 3 is used, so required array size = 4!

    // check if pulsewidths 2 + 3 look like servo pulses
    for (int i = 2; i < 4; i++)
    {
        if (pulseWidth[i] > pulseMinLimit[i] && pulseWidth[i] < pulseMaxLimit[i])
        {
            if (pulseWidth[i] < pulseMin[i])
                pulseWidth[i] = pulseMin[i]; // Constrain the value
            if (pulseWidth[i] > pulseMax[i])
                pulseWidth[i] = pulseMax[i];

            // calculate a throttle value from the pulsewidth signal
            if (pulseWidth[i] > pulseMaxNeutral[i])
            {
                currentThrottleLR[i] = map(pulseWidth[i], pulseMaxNeutral[i], pulseMax[i], 0, 500);
            }
            else if (pulseWidth[i] < pulseMinNeutral[i])
            {
                currentThrottleLR[i] = map(pulseWidth[i], pulseMinNeutral[i], pulseMin[i], 0, 500);
            }
            else
            {
                currentThrottleLR[i] = 0;
            }
        }
    }

    // Mixing both sides together (take the bigger value)
    currentThrottle = max(currentThrottleLR[2], currentThrottleLR[3]);

    // Print debug infos
    static unsigned long printTrackedMillis;
#ifdef TRACKED_DEBUG // can slow down the playback loop!
    if (millis() - printTrackedMillis > 1000)
    { // Every 1000ms
        printTrackedMillis = millis();

        Serial.printf("TRACKED DEBUG:\n");
        Serial.printf("currentThrottleLR[2]: %i\n", currentThrottleLR[2]);
        Serial.printf("currentThrottleLR[3]: %i\n", currentThrottleLR[3]);
        Serial.printf("currentThrottle: %i\n", currentThrottle);
    }
#endif // TRACKED_DEBUG

#else // Normal mode ---------------------------------------------------------------------------

    // check if the pulsewidth looks like a servo pulse
    if (tractionValue > pulseMinLimit3 && tractionValue < pulseMaxLimit3)
    {
        if (tractionValue < pulseMin3)
            tractionValue = pulseMin3; // Constrain the value
        if (tractionValue > pulseMax3)
            tractionValue = pulseMax3;

        // calculate a throttle value from the pulsewidth signal
        if (tractionValue > pulseMaxNeutral3)
        {
            currentThrottle = map(tractionValue, pulseMaxNeutral3, pulseMax3, 0, 500);
        }
        else if (tractionValue < pulseMinNeutral3)
        {
            currentThrottle = map(tractionValue, pulseMinNeutral3, pulseMin3, 0, 500);
        }
        else
        {
            currentThrottle = 0;
        }
    }
#endif

    // Auto throttle --------------------------------------------------------------------------
#if not defined EXCAVATOR_MODE
    // Auto throttle while gear shifting (synchronizing the Tamiya 3 speed gearbox)
    if (!escIsBraking && escIsDriving && shiftingAutoThrottle && !automatic && !doubleClutch)
    {
        if (gearUpShiftingInProgress && !doubleClutchInProgress)
            currentThrottle = 0; // No throttle
        if (gearDownShiftingInProgress || doubleClutchInProgress)
            currentThrottle = 500;                            // Full throttle
        currentThrottle = constrain(currentThrottle, 0, 500); // Limit throttle range
    }
#endif

    // Volume calculations --------------------------------------------------------------------------

    // As a base for some calculations below, fade the current throttle to make it more natural
    static unsigned long throttleFaderMicros;
    // static boolean blowoffLock;
    if (micros() - throttleFaderMicros > 500)
    { // Every 0.5ms
        throttleFaderMicros = micros();

        if (currentThrottleFaded < currentThrottle && !escIsBraking && currentThrottleFaded < 499)
            currentThrottleFaded += 2;
        if ((currentThrottleFaded > currentThrottle || escIsBraking) && currentThrottleFaded > 2)
            currentThrottleFaded -= 2;

        // Calculate throttle dependent engine idle volume
        if (!escIsBraking && !brakeDetect && engineRunning)
            throttleDependentVolume = map(currentThrottleFaded, 0, 500, engineIdleVolumePercentage, fullThrottleVolumePercentage);
        // else throttleDependentVolume = engineIdleVolumePercentage; // TODO
        else
        {
            if (throttleDependentVolume > engineIdleVolumePercentage)
                throttleDependentVolume--;
            else
                throttleDependentVolume = engineIdleVolumePercentage;
        }

        // Calculate throttle dependent engine rev volume
        if (!escIsBraking && !brakeDetect && engineRunning)
            throttleDependentRevVolume = map(currentThrottleFaded, 0, 500, engineRevVolumePercentage, fullThrottleVolumePercentage);
        // else throttleDependentRevVolume = engineRevVolumePercentage; // TODO
        else
        {
            if (throttleDependentRevVolume > engineRevVolumePercentage)
                throttleDependentRevVolume--;
            else
                throttleDependentRevVolume = engineRevVolumePercentage;
        }

        // Calculate throttle dependent Diesel knock volume
        if (!escIsBraking && !brakeDetect && engineRunning && (currentThrottleFaded > dieselKnockStartPoint))
            throttleDependentKnockVolume = map(currentThrottleFaded, dieselKnockStartPoint, 500, dieselKnockIdleVolumePercentage, 100);
        // else throttleDependentKnockVolume = dieselKnockIdleVolumePercentage;
        else
        {
            if (throttleDependentKnockVolume > dieselKnockIdleVolumePercentage)
                throttleDependentKnockVolume--;
            else
                throttleDependentKnockVolume = dieselKnockIdleVolumePercentage;
        }

        // Calculate engine rpm dependent jake brake volume
        if (engineRunning)
            rpmDependentJakeBrakeVolume = map(currentRpm, 0, 500, jakeBrakeIdleVolumePercentage, 100);
        else
            rpmDependentJakeBrakeVolume = jakeBrakeIdleVolumePercentage;

#if defined RPM_DEPENDENT_KNOCK // knock volume also depending on engine rpm
        // Calculate RPM dependent Diesel knock volume
        if (currentRpm > 400)
            rpmDependentKnockVolume = map(currentRpm, knockStartRpm, 500, minKnockVolumePercentage, 100);
        else
            rpmDependentKnockVolume = minKnockVolumePercentage;
#endif

        // Calculate engine rpm dependent turbo volume
        if (engineRunning)
            throttleDependentTurboVolume = map(currentRpm, 0, 500, turboIdleVolumePercentage, 100);
        else
            throttleDependentTurboVolume = turboIdleVolumePercentage;

        // Calculate engine rpm dependent cooling fan volume
        if (engineRunning && (currentRpm > fanStartPoint))
            throttleDependentFanVolume = map(currentRpm, fanStartPoint, 500, fanIdleVolumePercentage, 100);
        else
            throttleDependentFanVolume = fanIdleVolumePercentage;

        // Calculate throttle dependent supercharger volume
        if (!escIsBraking && !brakeDetect && engineRunning && (currentRpm > chargerStartPoint))
            throttleDependentChargerVolume = map(currentThrottleFaded, chargerStartPoint, 500, chargerIdleVolumePercentage, 100);
        else
            throttleDependentChargerVolume = chargerIdleVolumePercentage;

        // Calculate engine rpm dependent wastegate volume
        if (engineRunning)
            rpmDependentWastegateVolume = map(currentRpm, 0, 500, wastegateIdleVolumePercentage, 100);
        else
            rpmDependentWastegateVolume = wastegateIdleVolumePercentage;
    }

    // Calculate engine load (used for torque converter slip simulation)
    engineLoad = currentThrottle - currentRpm;

    if (engineLoad < 0 || escIsBraking || brakeDetect)
        engineLoad = 0; // Range is 0 - 180
    if (engineLoad > 180)
        engineLoad = 180;

    // Additional sounds volumes -----------------------------

    // Tire squealing ----
    uint8_t steeringAngle = 0;

    // Cornering squealing
    if (steeringValue < 1500)
        steeringAngle = map(steeringValue, 1000, 1500, 100, 0);
    else if (steeringValue > 1500)
        steeringAngle = map(steeringValue, 1500, 2000, 0, 100);
    else
        steeringAngle = 0;

    tireSquealVolume = steeringAngle * currentSpeed * currentSpeed / 125000; // Volume = steering angle * speed * speed

    // Brake squealing
    if ((driveState == 2 || driveState == 4) && currentSpeed > 50 && currentThrottle > 250)
    {
        tireSquealVolume += map(currentThrottle, 250, 500, 0, 100);
    }

    tireSquealVolume = constrain(tireSquealVolume, 0, 100);
}

#endif // __ENGINE_H__