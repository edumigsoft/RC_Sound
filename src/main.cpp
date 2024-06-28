#include <Arduino.h>

#include "rom/rtc.h"
#include "soc/rtc_wdt.h"

#include "ps3.h"
#include "battery.h"
#include "leds.h"
#include "triggers.h"
#include "playback.h"

// static unsigned long dacOffsetMicros;
// boolean dacInit;

// void dacOffsetFade()
// {
//   if (!dacInit)
//   {
//     if (micros() - dacOffsetMicros > 100)
//     { // Every 0.1ms
//       dacOffsetMicros = micros();
//       dacOffset++; // fade DAC offset slowly to prevent it from popping, if ESP32 powered up after amplifier
//       if (dacOffset == 128)
//         dacInit = true;
//     }
//   }
// }

void engineOnOff()
{
  // static unsigned long pulseDelayMillis; // TODO
  static unsigned long idleDelayMillis;

  // Engine automatically switched on or off depending on throttle position and 15s delay timne
  if (currentThrottle > 80 || driveState != 0)
    idleDelayMillis = millis(); // reset delay timer, if throttle not in neutral

#ifdef AUTO_ENGINE_ON_OFF
  if (millis() - idleDelayMillis > 15000)
  {
    engineOn = false; // after delay, switch engine off
  }
#endif

#ifdef AUTO_LIGHTS
  if (millis() - idleDelayMillis > 10000)
  {
    lightsOn = false; // after delay, switch light off
  }
#endif

  // Engine start detection
  if (currentThrottle > 100 && !airBrakeTrigger)
  {
    engineOn = true;

#ifdef AUTO_LIGHTS
    lightsOn = true;
#endif
  }
}

void Task1code(void *pvParameters)
{
  for (;;)
  {
    // DAC offset fader
    dacOffsetFade();

    // if (xSemaphoreTake(xRpmSemaphore, portMAX_DELAY))
    // {
    //   // Simulate engine mass, generate RPM signal
    //   engineMassSimulation();

    //   // Call gear selector
    //   if (automatic || doubleClutch)
    //     automaticGearSelector();
    //   xSemaphoreGive(xRpmSemaphore); // Now free or "Give" the semaphore for others.
    // }

    // Switch engine on or off
    engineOnOff();

    // LED control
    // if (autoZeroDone)
    // led();

    // Shaker control
    // shaker();

    // Gearbox detection
    // gearboxDetection();

    // ESC control & low discharge protection
    // esc();
  }
}

void setup()
{
  disableCore0WDT();

  rtc_wdt_protect_off();
  rtc_wdt_set_length_of_reset_signal(RTC_WDT_SYS_RESET_SIG, RTC_WDT_LENGTH_3_2us);
  rtc_wdt_set_stage(RTC_WDT_STAGE0, RTC_WDT_STAGE_ACTION_RESET_SYSTEM);
  rtc_wdt_set_time(RTC_WDT_STAGE0, 10000);
  rtc_wdt_enable();
  // rtc_wdt_disable();
  rtc_wdt_protect_on();

  // Serial.begin(MONITOR_BOUND);

  // setupPs3();

  battery.attach(BATTERY_DETECT_PIN);

  delay(1000);

  if (xPwmSemaphore == NULL)
  {
    xPwmSemaphore = xSemaphoreCreateMutex();
    if ((xPwmSemaphore) != NULL)
      xSemaphoreGive((xPwmSemaphore));
  }

  if (xRpmSemaphore == NULL)
  {
    xRpmSemaphore = xSemaphoreCreateMutex();
    if ((xRpmSemaphore) != NULL)
      xSemaphoreGive((xRpmSemaphore));
  }

  setupStatusLED();

  setupBattery();

  indicatorL.on();
  indicatorR.on();

  setupPs3();

  // setupMcpwm(); // mcpwm servo output setup

  // Refresh sample intervals (important, because MAX_RPM_PERCENTAGE was probably changed above)
  // maxSampleInterval = 4000000 / sampleRate;
  // minSampleInterval = 4000000 / sampleRate * 100 / MAX_RPM_PERCENTAGE;

  // Task 1 setup (running on core 0)
  TaskHandle_t Task1;
  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code, // Task function
      "Task1",   // name of task
      8192,      // Stack size of task (8192)
      NULL,      // parameter of the task
      1,         // priority of the task (1 = low, 3 = medium, 5 = highest)
      &Task1,    // Task handle to keep track of created task
      0);        // pin task to core 0

  // Interrupt timer for variable sample rate playback
  variableTimer = timerBegin(0, 20, true);                           // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 20 -> 250 ns = 0.25 us, countUp
  timerAttachInterrupt(variableTimer, &variablePlaybackTimer, true); // edge (not level) triggered
  timerAlarmWrite(variableTimer, variableTimerTicks, true);          // autoreload true
  timerAlarmEnable(variableTimer);                                   // enable

  // Interrupt timer for fixed sample rate playback
  fixedTimer = timerBegin(1, 20, true);                        // timer 1, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 20 -> 250 ns = 0.25 us, countUp
  timerAttachInterrupt(fixedTimer, &fixedPlaybackTimer, true); // edge (not level) triggered
  timerAlarmWrite(fixedTimer, fixedTimerTicks, true);          // autoreload true
  timerAlarmEnable(fixedTimer);                                // enable

  rtc_wdt_feed(); // Feed watchdog timer

  // autoZeroDone = true;

  // ESC output range calibration
  // escPulseMaxNeutral = pulseZero[3] + escTakeoffPunch; // Additional takeoff punch around zero
  // escPulseMinNeutral = pulseZero[3] - escTakeoffPunch;

  // escPulseMax = pulseZero[3] + escPulseSpan;
  // escPulseMin = pulseZero[3] - escPulseSpan + escReversePlus; // Additional power for ESC with slow reverse

  // ESC setup
  // setupMcpwmESC(); // ESC now using mpcpwm
}

void loop()
{
  // if (!Ps3.isConnected())
  // {
  //   engineOn = false;
  //   return;
  // }

  // PWM servo signal output
  // mcpwmOutput();

  // Horn triggering
  triggerHorn();

  // Indicator (turn signal) triggering
  triggerIndicators();

  if (xSemaphoreTake(xRpmSemaphore, portMAX_DELAY))
  {
    // Map pulsewidth to throttle
    // mapThrottle();

    xSemaphoreGive(xRpmSemaphore); // Now free or "Give" the semaphore for others.
  }

  // Feeding the RTC watchtog timer is essential!
  rtc_wdt_feed();
}
