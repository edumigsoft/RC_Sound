#ifndef __PS3_H__
#define __PS3_H__

#include "Ps3Controller.h"
#include "variables.h"
// #include "leds.h"

volatile int gear = 0;

void Ps3Notify()
{
    if (Ps3.event.button_down.start)
    {
        engineOn = !engineOn;
    }

    if (Ps3.event.button_down.circle)
    {
        hornTrigger = true;
        hornLatch = true;
    }
    else
    {
        hornTrigger = false;
    }

    if (Ps3.event.button_down.triangle)
    {
        hazard = !hazard;
    }

    if (Ps3.event.button_down.cross)
    {
        sirenTrigger = !sirenTrigger;
    }

    if (Ps3.event.button_down.square)
    {
        blueLightTrigger = !blueLightTrigger;
    }

    // if (abs(Ps3.event.analog_changed.button.l1))
    // {
    //   Serial.print("Pressing the left shoulder button: ");
    //   Serial.println(Ps3.data.analog.button.l1, DEC);
    // }

    // if (Ps3.event.button_down.l1)
    //   Serial.println("Started pressing the left shoulder button");
    // if (Ps3.event.button_up.l1)
    //   Serial.println("Released the left shoulder button");

    if (Ps3.event.button_down.l1)
    {
        indicatorLonManual = !indicatorLonManual;
    }

    if (Ps3.event.button_down.r1)
    {
        indicatorRonManual = !indicatorRonManual;
    }

    int yAxisValue = (Ps3.data.analog.stick.ly); // Left stick  - y axis - forward/backward car movement

    if (yAxisValue == 0)
    {
        yAxisValue = map(Ps3.data.analog.button.l2, 0, 255, 0, -128);

        if (yAxisValue == 0)
        {
            yAxisValue = map(Ps3.data.analog.button.r2, 0, 255, 0, 127);
        }
    }

    int xAxisValue = (Ps3.data.analog.stick.rx); // Right stick - x axis - left/right car movement

    if (engineOn)
    {
        steeringAngle = map(xAxisValue, -128, 127, STEERING_ANGLE_LEFT, STEERING_ANGLE_RIGHT);

#ifndef SEMI_AUTOMATIC
        if (Ps3.event.button_down.up)
        {
            selectedGear++;
            if (selectedGear == 4)
            {
                selectedGear = 3;
            }
        }

        if (Ps3.event.button_down.down)
        {
            selectedGear--;
            if (selectedGear == 0)
            {
                selectedGear = 1;
            }
        }
#else
//
#endif

        currentThrottle = map(yAxisValue, 127, -128, 0, 500);
    }
}

void Ps3OnConnect()
{
    // engineStart = true;
    Serial.println("Connected!.");
}

void Ps3OnDisConnect()
{
    // engineStart = false;
    engineOn = false;
    Serial.println("DisConnected!.");
}

void setupPs3()
{
    Ps3.attach(Ps3Notify);
    Ps3.attachOnConnect(Ps3OnConnect);
    Ps3.attachOnDisconnect(Ps3OnDisConnect);
    // Change according to control mac
    Ps3.begin("5C:C9:D3:7D:A3:29");

    // Blink leds no connected on
    while (!Ps3.isConnected())
    {
        Serial.println("Waiting for connection to control");
        delay(500);
    }
    // Blink leds no connected off

    // indicatorL.flash(70, 75, 500, 3); // Show 3 fast flashes on indicators!
    // indicatorR.flash(70, 75, 500, 3);
    // rtc_wdt_feed(); // Feed watchdog timer
}

#endif // __PS3_H__