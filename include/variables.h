#ifndef __VARIABLES_H__
#define __VARIABLES_H__

/*
Variables or consts choose car
*/

#include "vehicle.h"

/*************** */

// #define JAKEBRAKE_ENGINE_SLOWDOWN
// #define JAKE_BRAKE_SOUND
// #define REV_SOUND
#define SEMI_AUTOMATIC
#define AUTO_INDICATORS

#include <Arduino.h>

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

// Direction
int steeringAngle = STEERING_ANGLE_MIDDLE;

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

// ESC
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
// uint16_t escRampTime;
// int8_t direction = 0;
static uint16_t escPulseWidth = 1500;
static uint16_t escPulseWidthOut = 1500;
static uint16_t escSignal = 1500;
// static uint8_t motorDriverDuty = 0;
static unsigned long escMillis;
// static unsigned long lastStateTime;
// // static int8_t pulse; // -1 = reverse, 0 = neutral, 1 = forward
// // static int8_t escPulse; // -1 = reverse, 0 = neutral, 1 = forward
// static int8_t driveRampRate;
// static int8_t driveRampGain;
// static int8_t brakeRampRate;
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

// Battery
// float batteryCutoffvoltage;
// float batteryVoltage;
// uint8_t numberOfCells;
bool batteryProtection = false;

// Clutch
boolean clutchDisengaged = true; // Active while clutch is disengaged

#endif // __VARIABLES_H__