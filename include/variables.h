#ifndef __VARIABLES_H__
#define __VARIABLES_H__

#include <Arduino.h>
#include "vehicle.h"

// Sound
volatile boolean engineOn = false;                // Signal for engine on / off
volatile boolean engineStart = false;             // Active, if engine is starting up
volatile boolean engineRunning = false;           // Active, if engine is running
volatile boolean engineStop = false;              // Active, if engine is shutting down
volatile boolean jakeBrakeRequest = false;        // Active, if engine jake braking is requested
volatile boolean engineJakeBraking = false;       // Active, if engine is jake braking
volatile boolean wastegateTrigger = false;        // Trigger wastegate (blowoff) after rapid throttle drop
volatile boolean blowoffTrigger = false;          // Trigger jake brake sound (blowoff) after rapid throttle drop
volatile boolean dieselKnockTrigger = false;      // Trigger Diesel ignition "knock"
volatile boolean dieselKnockTriggerFirst = false; // The first Diesel ignition "knock" per sequence
volatile boolean airBrakeTrigger = false;         // Trigger for air brake noise
volatile boolean parkingBrakeTrigger = false;     // Trigger for air parking brake noise
volatile boolean shiftingTrigger = false;         // Trigger for shifting noise
volatile boolean hornTrigger = false;             // Trigger for horn on / off
volatile boolean sirenTrigger = false;            // Trigger for siren  on / off
volatile boolean sound1trigger = false;           // Trigger for sound1  on / off
volatile boolean couplingTrigger = false;         // Trigger for trailer coupling  sound
volatile boolean uncouplingTrigger = false;       // Trigger for trailer uncoupling  sound
// volatile boolean bucketRattleTrigger = false;     // Trigger for bucket rattling  sound
volatile boolean indicatorSoundOn = false; // active, if indicator bulb is on
// volatile boolean outOfFuelMessageTrigger = false; // Trigger for out of fuel message

// Sound latches
volatile boolean hornLatch = false;  // Horn latch bit
volatile boolean sirenLatch = false; // Siren latch bit

// Sound volumes
volatile uint16_t throttleDependentVolume = 0;      // engine volume according to throttle position
volatile uint16_t throttleDependentRevVolume = 0;   // engine rev volume according to throttle position
volatile uint16_t rpmDependentJakeBrakeVolume = 0;  // Engine rpm dependent jake brake volume
volatile uint16_t throttleDependentKnockVolume = 0; // engine Diesel knock volume according to throttle position
// volatile uint16_t rpmDependentKnockVolume = 0;        // engine Diesel knock volume according to engine RPM
volatile uint16_t throttleDependentTurboVolume = 0;   // turbo volume according to rpm
volatile uint16_t throttleDependentFanVolume = 0;     // cooling fan volume according to rpm
volatile uint16_t throttleDependentChargerVolume = 0; // cooling fan volume according to rpm
volatile uint16_t rpmDependentWastegateVolume = 0;    // wastegate volume according to rpm
volatile uint16_t tireSquealVolume = 0;               // Tire squeal volume according to speed and cornering radius
volatile int16_t masterVolume = 100;                  // Master volume percentage
// Volume adjustment
// const  uint8_t numberOfVolumeSteps = 3; // The mumber of volume steps below
// const uint8_t masterVolumePercentage[] = {100, 66, 44}; // loud, medium, silent (more than 100% may cause distortions)
const uint8_t numberOfVolumeSteps = 4;                     // The mumber of volume steps below
const uint8_t masterVolumePercentage[] = {100, 66, 44, 0}; // loud, medium, silent, no sound (more than 100% may cause distortions)
// Crawler mode
const uint8_t masterVolumeCrawlerThreshold = 44; // If master volume is <= this threshold, crawler mode (without virtual inertia) is active

// Lights
int8_t lightsState = 0;                        // for lights state machine
volatile boolean lightsOn = false;             // Lights on
volatile boolean headLightsFlasherOn = false;  // Headlights flasher impulse (Lichthupe)
volatile boolean headLightsHighBeamOn = false; // Headlights high beam (Fernlicht)
volatile boolean blueLightTrigger = false;     // Bluelight on (Blaulicht)
boolean indicatorLon = false;                  // Left indicator (Blinker links)
boolean indicatorLonManual = false;            // Left indicator manual
boolean indicatorRon = false;                  // Right indicator (Blinker rechts)
boolean indicatorRonManual = false;            // Right indicator manual
boolean fogLightOn = false;                    // Fog light is on
boolean cannonFlash = false;                   // Flashing cannon fire
boolean hazard;
uint8_t crankingDim;
uint8_t dipDim;
uint8_t xenonIgnitionFlash;
static unsigned long xenonMillis;
uint32_t indicatorFade = 300; // 300 is the fading time, simulating an incandescent bulb
// Adjust the brightness of your lights here -----------------------------------------------------------------------------------------------------
// All brightness values 0 - 255
uint8_t cabLightsBrightness = 100; // Usually 255, 100 for Actros & Ural
uint8_t sideLightsBrightness = 150; // Usually 200, 100 for WPL C44, 50 for Landy, 100 for P407, 150 for Actros
uint8_t rearlightDimmedBrightness = 30; // tailligt brightness, if not braking, about 30
uint8_t rearlightParkingBrightness = 3; // 0, if you want the taillights being off, if side lights are on, or about 5 if you want them on (0 for US Mode)
uint8_t headlightParkingBrightness = 3; // 0, if you want the headlights being off, if side lights are on, or about 5 if you want them on (0 for US Mode)
uint8_t reversingLightBrightness = 140; // Around 140, 50 for Landy & Ural

// Transmission
uint8_t selectedGear = 1;             // The currently used gear of our shifting gearbox
uint8_t selectedAutomaticGear = 1;    // The currently used gear of our automatic gearbox
boolean gearUpShiftingInProgress;     // Active while shifting upwards
boolean doubleClutchInProgress;       // Double-clutch (Zwischengas)
boolean gearDownShiftingInProgress;   // Active while shifting downwards
boolean gearUpShiftingPulse;          // Active, if shifting upwards begins
boolean gearDownShiftingPulse;        // Active, if shifting downwards begins
volatile boolean neutralGear = false; // Transmission in neutral
boolean lowRange = false;             // Transmission range (off road reducer)
// In some cases we want a different reverse acceleration for automatic transmission vehicles.
uint16_t automaticReverseAccelerationPercentage = 100;
// Low range percentage is used for MODE1_SHIFTING (off road reducer)
uint16_t lowRangePercentage = 58; // WPL 2 speed ratios = 29:1, 17:1 = 58% in low range. You may want to change this for other 2 speed transmissions

// Direction
int16_t steeringValue = 1500;

// Throttle
int16_t currentThrottle = 0;      // 0 - 500 (Throttle trigger input)
int16_t currentThrottleFaded = 0; // faded throttle for volume calculations etc.

// Semaphore Handles
SemaphoreHandle_t xPwmSemaphore;
SemaphoreHandle_t xRpmSemaphore;

// Task main
TaskHandle_t Task1;

//
boolean unlock5thWheel;
boolean winchPull;
boolean winchRelease;
boolean winchEnabled;

// DAC
static unsigned long dacOffsetMicros;
boolean dacInit;
volatile uint8_t dacOffset = 0; // 128, but needs to be ramped up slowly to prevent popping noise, if switched on

// TRACTION
volatile boolean escIsBraking = false; // ESC is in a braking state
volatile boolean escIsDriving = false; // ESC is in a driving state
volatile boolean escInReverse = false; // ESC is driving or braking backwards
volatile boolean brakeDetect = false;  // Additional brake detect signal, enabled immediately, if brake applied
int8_t driveState = 0;                 // for ESC state machine
uint16_t escPulseMax = 2000;           // ESC calibration variables (values will be changed later)
uint16_t escPulseMin = 1000;
uint16_t escPulseMaxNeutral = 1500;
uint16_t escPulseMinNeutral = 1500;
uint16_t currentSpeed = 0;         // 0 - 500 (current ESC power)
volatile bool crawlerMode = false; // Crawler mode intended for crawling competitons (withouth sound and virtual inertia)
static int8_t driveRampRate;
static int8_t driveRampGain;
static int8_t brakeRampRate;
static uint16_t escPulseWidth = 1500;
static uint16_t escPulseWidthOut = 1500;
// static uint16_t escSignal = 1500;
static unsigned long escMillis;
// static unsigned long lastStateTime;
const uint16_t pulseNeutral = 30;
const uint16_t pulseSpan = 480;
const uint16_t pulseZero3 = 1500;
const uint16_t pulseLimit = 1100;
uint16_t escRampTime;
uint16_t pulseMaxNeutral3 = pulseZero3 + pulseNeutral;
uint16_t pulseMinNeutral3 = pulseZero3 - pulseNeutral;
uint16_t pulseMaxLimit3 = pulseZero3 + pulseLimit;
uint16_t pulseMinLimit3 = pulseZero3 - pulseLimit;
uint16_t tractionValue = pulseZero3;
uint16_t pulseMax3 = pulseZero3 + pulseSpan;
uint16_t pulseMin3 = pulseZero3 - pulseSpan;
// Allows to scale vehicle file dependent acceleration
uint16_t globalAccelerationPercentage = 100; // about 100 - 200% (200 for Jeep, 150 for 1/8 Landy) Experimental, may cause automatic transmission shifting issues!
// Crawler mode escRampTime (see "8_Sound.h") WARNING: a very low setting may damage your transmission!
const uint8_t crawlerEscRampTime = 10; // about 10 (15 for Jeep), less = more direct control = less virtual inertia
// Brake margin: (Experimental!)
// This setting prevents the ESC from going completely back to zero / neutral as long as the brake trigger is pulled.
// This prevents the vehicle from rolling back as long as brake is applied. 0 = no effect, ca. 20 = strong effect.
// How it works? Prevents the ESC from entering the "drag brake range"
// Warning: vehicle may be unable to stop, if too high, especially when driving downhill! NEVER more than 20!
const uint16_t brakeMargin = 10; // For RZ7886 motor driver and 370 motor = 10
static uint8_t motorDriverDuty = 0;

// Sampling intervals for interrupt timer (adjusted according to your sound file sampling rate)
uint32_t maxSampleInterval = 4000000 / sampleRate;
uint32_t minSampleInterval = 4000000 / sampleRate * 100 / MAX_RPM_PERCENTAGE;

// Interrupt timer for variable sample rate playback (engine sound)
hw_timer_t *variableTimer = NULL;
portMUX_TYPE variableTimerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t variableTimerTicks = maxSampleInterval;

// Interrupt timer for fixed sample rate playback (horn etc., playing in parallel with engine sound)
hw_timer_t *fixedTimer = NULL;
portMUX_TYPE fixedTimerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t fixedTimerTicks = maxSampleInterval;

// Engine
const int16_t maxRpm = 500;       // always 500
const int16_t minRpm = 0;         // always 0
int32_t currentRpm = 0;           // 0 - 500 (signed required!)
volatile uint8_t engineState = 0; // Engine state
enum EngineState                  // Engine state enum
{
    OFF,      // Engine is off
    STARTING, // Engine is starting
    RUNNING,  // Engine is running
    STOPPING, // Engine is stopping
    PARKING_BRAKE
};
int16_t engineLoad = 0;                 // 0 - 500
volatile uint16_t engineSampleRate = 0; // Engine sample rate
int32_t speedLimit = maxRpm;            // The speed limit, depending on selected virtual gear

// Shaker parameters (simulating engine vibrations)
const uint8_t shakerStart = 100; // Shaker power while engine start (max. 255, about 100)
const uint8_t shakerIdle = 49; // Shaker power while idling (max. 255, about 49)
const uint8_t shakerFullThrottle = 40; // Shaker power while full throttle (max. 255, about 40)
const uint8_t shakerStop = 60; // Shaker power while engine stop (max. 255, about 60)

// Battery
// float batteryCutoffvoltage;
// float batteryVoltage;
// uint8_t numberOfCells;
bool batteryProtection = false;
/* Battery low discharge protection (only for boards with voltage divider resistors):
 *  IMPORTANT: Enter used resistor values in Ohms (Ω) and THEN adjust DIODE_DROP, until your readings match the actual battery voltage! */
// //#define BATTERY_PROTECTION // This will disable the ESC output, if the battery cutout voltage is reached. 2 fast flashes = battery error!
// const float CUTOFF_VOLTAGE = 3.3;        // Usually 3.3 V per LiPo cell. NEVER below 3.2 V!
// const float FULLY_CHARGED_VOLTAGE = 4.2; // Usually 4.2 V per LiPo cell, NEVER above!
// const float RECOVERY_HYSTERESIS = 0.2;   // around 0.2 V
/* Note on resistor values: These values will be used to calculate the actual ratio between these two resistors (which is also called a "voltage divider").
 * When selecting resistors, always use two of the same magnitude: Like, for example, 10k/2k, 20k/4k or 100k/20k. NEVER exceed a ratio LOWER than (4:1 = 4)!
 * WARNING: If the ratio is too LOW, like 10k/5k (2:1 = 2), the battery voltage will most likely DAMAGE the controller permanently!
 * Example calculation: 2000 / (2000 + 10000) = 0.166 666 666 7; 7.4 V * 0.167 = 1.2358 V (of 3.3 V maximum on GPIO Pin). */
// uint32_t RESISTOR_TO_BATTTERY_PLUS = 10000; // Value in Ohms (Ω), for example 10000
// uint32_t RESISTOR_TO_GND = 1000;           // Value in Ohms (Ω), for example 2000. Measuring exact resistor values before soldering, if possible is recommended!
// float DIODE_DROP = 0;                   // Fine adjust measured value and/or consider diode voltage drop (about 0.34V for SS34 diode)
/* It is recommended to add a sticker to your ESP32, which includes the 3 calibration values above */
// volatile int outOfFuelVolumePercentage = 80; // Adjust the message volume in %
// Select the out of fuel message you want:
// #include "vehicles/sounds/OutOfFuelEnglish.h"
// // #include "vehicles/sounds/OutOfFuelGerman.h"
// // #include "vehicles/sounds/OutOfFuelFrench.h"
// // #include "vehicles/sounds/OutOfFuelDutch.h"
// // #include "vehicles/sounds/OutOfFuelSpanish.h"
// // #include "vehicles/sounds/OutOfFuelPortuguese.h"
// // #include "vehicles/sounds/OutOfFuelJapanese.h"
// // #include "vehicles/sounds/OutOfFuelChinese.h"
// // #include "vehicles/sounds/OutOfFuelTurkish.h"
// // #include "vehicles/sounds/OutOfFuelRussian.h"

// Clutch
boolean clutchDisengaged = true; // Active while clutch is disengaged
// Clutch options ==========================================================================================================================
uint16_t maxClutchSlippingRpm = 250; // The clutch will never slip above this limit! (about 250) 500 for vehicles like locomotives
// and the Kirovets tractor with hydrostatic or electric drive! Mainly required for "VIRTUAL_3_SPEED" mode

#endif // __VARIABLES_H__