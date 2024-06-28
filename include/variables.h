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
volatile boolean engineOn = false;        // Signal for engine on / off
volatile boolean engineStart = false;     // Active, if engine is starting up
volatile boolean engineRunning = false;   // Active, if engine is running
volatile boolean engineStop = false;      // Active, if engine is shutting down
volatile boolean hornTrigger = false;     // Trigger for horn on / off
volatile boolean sirenTrigger = false;    // Trigger for siren  on / off
volatile boolean airBrakeTrigger = false; // Trigger for air brake noise

// Sound latches
volatile boolean hornLatch = false;  // Horn latch bit
volatile boolean sirenLatch = false; // Siren latch bit

// Sound volumes
volatile uint16_t throttleDependentVolume = 0; // engine volume according to throttle position
// volatile uint16_t throttleDependentRevVolume = 0;     // engine rev volume according to throttle position
// volatile uint16_t rpmDependentJakeBrakeVolume = 0;    // Engine rpm dependent jake brake volume
// volatile uint16_t throttleDependentKnockVolume = 0;   // engine Diesel knock volume according to throttle position
// volatile uint16_t rpmDependentKnockVolume = 0;        // engine Diesel knock volume according to engine RPM
// volatile uint16_t throttleDependentTurboVolume = 0;   // turbo volume according to rpm
// volatile uint16_t throttleDependentFanVolume = 0;     // cooling fan volume according to rpm
// volatile uint16_t throttleDependentChargerVolume = 0; // cooling fan volume according to rpm
// volatile uint16_t rpmDependentWastegateVolume = 0;    // wastegate volume according to rpm
// volatile uint16_t tireSquealVolume = 0;               // Tire squeal volume according to speed and cornering radius

// Lights
boolean hazard;
volatile boolean blueLightTrigger = false; // Bluelight on (Blaulicht)

boolean indicatorLon = false;       // Left indicator (Blinker links)
boolean indicatorLonManual = false; // Left indicator manual
boolean indicatorRon = false;       // Right indicator (Blinker rechts)
boolean indicatorRonManual = false; // Right indicator manual

// Transmission
uint8_t selectedGear = 1; // The currently used gear of our shifting gearbox
// uint8_t selectedAutomaticGear = 1; // The currently used gear of our automatic gearbox

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
// volatile int16_t masterVolume = 100; // Master volume percentage
volatile uint8_t dacOffset = 0; // 128, but needs to be ramped up slowly to prevent popping noise, if switched on

// ESC
int8_t driveState = 0; // for ESC state machine

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

#endif // __VARIABLES_H__