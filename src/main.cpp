#include <Arduino.h>

// #define DOUBLE_CLUTCH // Double-clutch (Zwischengas) Enable this for older manual transmission trucks without synchronised gears
// #define HIGH_SLIPPINGPOINT // Clutch will engage @ higher RPM, if defined. Comment this out for heavy vehicles like semi trucks
// Transmission controls options ===========================================================================================================
// #define SEMI_AUTOMATIC This will simulate a semi automatic transmission. Shifting is not controlled by the 3 position switch in this mode!
#define SEMI_AUTOMATIC // Works for VIRTUAL_3_SPEED or real 3 speed transmission. Don't select this @ the same time as VIRTUAL_16_SPEED_SEQUENTIAL
// #define MODE1_SHIFTING The 2 speed transmission is shifted by the "Mode 1" button instead of the 3 position switch.
//  This is often used in WPL vehicles with 2 speed transmission, used as off road reducer, shifted while driving slowly in order to engage properly.
// #define MODE1_SHIFTING
// #define TRANSMISSION_NEUTRAL Allows to put the transmission in neutral. This can't be used, if the "Mode 1" button is used for other stuff!
//  You can leave it on, if defined MODE1_SHIFTING. It is disabled automatically in this case.
#define TRANSMISSION_NEUTRAL
// #define VIRTUAL_3_SPEED allows to simulate a 3 speed shifing transmission, if your vehicle doesn't have a real one.
// Gears are virtually shifted, using the 3 position switch. Example: your crawler has a 2 speed transmission, which is used as off road reducer,
// but no real 3 speed shifting transmission. Don't uncomment if for vehicles with electric or hydrostatic drive or automatic transmissions!
// Also don't use it for STEAM_LOCOMOTIVE_MODE
// #define VIRTUAL_3_SPEED
// #define VIRTUAL_16_SPEED_SEQUENTIAL will enable a sequencial transmission, shifted by up / down impulses via 3 position switch
// #define VIRTUAL_16_SPEED_SEQUENTIAL // This is still experimental and not working properly! Don't use it.
// Additional transmission options =========================================================================================================
// Automatic transmission with overdrive (lower RPM in top gear, gear ratio lower than 1:1, 4 & 6 speed only)
// Also usable in combination with VIRTUAL_3_SPEED. The 4th gear is switched automatically in this case, if driving in 3rd gear @ full throttle
#define OVERDRIVE // Don't use it for: doubleClutch. Not working with SEMI_AUTOMATIC, but you can leave it on in this case.
// #define NO_SIREN // siren sound is not played, if defined
// #define NO_INDICATOR_SOUND // If you don't want the indicator "tick - tack" sound
//#define NO_CABLIGHTS // The cablights step in the lights sequence is skipped, if defined
//#define NO_FOGLIGHTS // The foglights step in the lights sequence is skipped, if defined
#define THIRD_BRAKELIGHT // if defined, pin 32 is used for a third brake light, otherwise for a trailer presence switch (pulled to GND = no trailer attached)
//#define FLICKERING_WHILE_CRANKING // The lights will flicker a bit during engine cranking, otherwise they are just a bit dimmed
#define HAZARDS_WHILE_5TH_WHEEL_UNLOCKED // Hazards on, if 5th wheel is unlocked
// #define JAKEBRAKE_ENGINE_SLOWDOWN
// #define JAKE_BRAKE_SOUND
// #define REV_SOUND
#define AUTO_INDICATORS

#include "rom/rtc.h"
#include "soc/rtc_wdt.h"

#include "ps3.h"
#include "variables.h"
#include "battery.h"
#include "triggers.h"
#include "playback.h"
#include "engine.h"
#include "steering.h"
#include "traction.h"

void Task1code(void *pvParameters)
{
  for (;;)
  {
    // DAC offset fader
    dacOffsetFade();

    if (xSemaphoreTake(xRpmSemaphore, portMAX_DELAY))
    {
      // Simulate engine mass, generate RPM signal
      engineMassSimulation();

      // Call gear selector
      if (automatic || doubleClutch)
        automaticGearSelector();
      xSemaphoreGive(xRpmSemaphore); // Now free or "Give" the semaphore for others.
    }

    // Switch engine on or off
    engineOnOff();

    // LED control
    led();

    // Shaker control
    shaker();

    // Gearbox detection
    gearboxDetection();

    // Traction control & low discharge protection
    tractionOutput();
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

  Serial.begin(MONITOR_BOUND);

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

  setupPs3();

  setupSteering();

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

  setupTraction();
}

void loop()
{
  // Não funciona
  // Serial.printf("Connected? = %o\n", Ps3.isConnected());
  // if (!Ps3.isConnected())
  // {
  //   Serial.println("!Ps3.isConnected()");
  //   Ps3DisConnected();
  //   return;
  // }

  // Servo signal output
  steeringOutput();

  // Horn triggering
  triggerHorn();

  // Indicator (turn signal) triggering
  triggerIndicators();

  if (xSemaphoreTake(xRpmSemaphore, portMAX_DELAY))
  {
    // Map pulsewidth to throttle
    mapThrottle();

    xSemaphoreGive(xRpmSemaphore); // Now free or "Give" the semaphore for others.
  }

  // Feeding the RTC watchtog timer is essential!
  rtc_wdt_feed();
}
