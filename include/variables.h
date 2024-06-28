#ifndef __VARIABLES_H__
#define __VARIABLES_H__

// 
// #define JAKEBRAKE_ENGINE_SLOWDOWN
// #define JAKE_BRAKE_SOUND
// #define REV_SOUND
#define SEMI_AUTOMATIC
#define AUTO_INDICATORS
//

#include <Arduino.h>

// Sound
volatile boolean engineOn = false;     // Signal for engine on / off
volatile boolean hornTrigger = false;  // Trigger for horn on / off
volatile boolean sirenTrigger = false; // Trigger for siren  on / off
// Sound latches
volatile boolean hornLatch = false; // Horn latch bit
// Lights
boolean hazard;
// boolean indicatorLon = false;       // Left indicator (Blinker links)
boolean indicatorLonManual = false; // Left indicator manual
// boolean indicatorRon = false;       // Right indicator (Blinker rechts)
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
// 
#endif // __VARIABLES_H__