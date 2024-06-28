#ifndef __BATTERY_H__
#define __BATTERY_H__

#include <ESP32AnalogRead.h>

ESP32AnalogRead battery;

void setupBattery()
{
#if defined BATTERY_PROTECTION

    Serial.printf("Battery voltage: %.2f V\n", batteryVolts());
    Serial.printf("Cutoff voltage per cell: %.2f V\n", CUTOFF_VOLTAGE);
    Serial.printf("Fully charged voltage per cell: %.2f V\n", FULLY_CHARGED_VOLTAGE);

#define CELL_SETPOINT (CUTOFF_VOLTAGE - ((FULLY_CHARGED_VOLTAGE - CUTOFF_VOLTAGE) / 2))

    if (batteryVolts() <= CELL_SETPOINT * 2)
        numberOfCells = 1;
    if (batteryVolts() > CELL_SETPOINT * 2)
        numberOfCells = 2;
    if (batteryVolts() > CELL_SETPOINT * 3)
        numberOfCells = 3;
    if (batteryVolts() > FULLY_CHARGED_VOLTAGE * 3)
        numberOfCells = 4;
    batteryCutoffvoltage = CUTOFF_VOLTAGE * numberOfCells; // Calculate cutoff voltage for battery protection
    if (numberOfCells > 1 && numberOfCells < 4)
    { // Only 2S & 3S batteries are supported!
        Serial.printf("Number of cells: %i (%iS battery detected) Based on setpoint: %.2f V\n", numberOfCells, numberOfCells, (CELL_SETPOINT * numberOfCells));
        Serial.printf("Battery cutoff voltage: %.2f V (%i * %.2f V) \n", batteryCutoffvoltage, numberOfCells, CUTOFF_VOLTAGE);
        for (uint8_t beeps = 0; beeps < numberOfCells; beeps++)
        { // Number of beeps = number of cells in series
            tone(26, 3000, 4, 0);
            // tone(26, 3000, 4); // For platform = espressif32@4.3.0
            delay(200);
        }
    }
    else
    {
        Serial.printf("Error, no valid battery detected! Only 2S & 3S batteries are supported!\n");
        Serial.printf("REMOVE BATTERY, CONTROLLER IS LOCKED = 2 FAST FLASHES!\n");
        bool locked = true;
        for (uint8_t beeps = 0; beeps < 10; beeps++)
        { // Number of beeps = number of cells in series
            tone(26, 3000, 4, 0);
            // tone(26, 3000, 4); // For platform = espressif32@4.3.0
            delay(30);
        }
        while (locked)
        {
            // wait here forever!
            indicatorL.flash(70, 75, 500, 2); // Show 2 fast flashes on indicators!
            indicatorR.flash(70, 75, 500, 2);
            rtc_wdt_feed(); // Feed watchdog timer
        }
    }
#else
    Serial.printf("Warning, BATTERY_PROTECTION disabled! ESC with low discharge protection required!\n");
#endif
    Serial.printf("-------------------------------------\n");
}

#endif // __BATTERY_H__