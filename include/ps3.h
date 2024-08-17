#ifndef __PS3_H__
#define __PS3_H__

#include "Ps3Controller.h"
#include "variables.h"
#include "leds.h"

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
        headLightsHighBeamOn = !headLightsHighBeamOn;
    }

    if (Ps3.event.button_down.up)
    {
        lightsState++;

        if (lightsState == 4)
        {
            lightsState = 3;
        }
    }

    if (Ps3.event.button_down.down)
    {
        lightsState--;

        if (lightsState == -1)
        {
            lightsState = 0;
        }
    }

    if (Ps3.event.button_down.right)
    {
        indicatorRonManual = !indicatorRonManual;
        indicatorLonManual = false;
    }

    if (Ps3.event.button_down.left)
    {
        indicatorLonManual = !indicatorLonManual;
        indicatorRonManual = false;
    }

    if (engineOn)
    {
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

        if (xAxisValue == 0)
        {
            xAxisValue = map(Ps3.data.analog.button.l1, 0, 255, 0, -128);

            if (xAxisValue == 0)
            {
                xAxisValue = map(Ps3.data.analog.button.r1, 0, 255, 0, 127);
            }
        }

        steeringValue = map(xAxisValue, -128, 127, 1000, 2000);
        tractionValue = map(yAxisValue, 127, -128, 1000, 2000);

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

#endif
    }
}

void Ps3OnConnect()
{
    indicatorL.off();
    indicatorR.off();

    Serial.println("Connected!.");
}

void Ps3DisConnected()
{
    engineOn = false;

    indicatorL.on();
    indicatorR.on();

    Serial.println("DisConnected!.");
}

void Ps3OnDisConnect()
{
    Ps3DisConnected();
}

void setupPs3()
{
    indicatorL.on();
    indicatorR.on();
    
    Ps3.attach(Ps3Notify);
    Ps3.attachOnConnect(Ps3OnConnect);
    Ps3.attachOnDisconnect(Ps3OnDisConnect);
    // Change according to control mac
    Ps3.begin(MAC_PS3);

    while (!Ps3.isConnected())
    {
        indicatorL.on();
        indicatorR.on();

        Serial.println("Waiting for connection to control");
        delay(300);

        indicatorL.off();
        indicatorR.off();

        delay(300);
    }
}

#endif // __PS3_H__