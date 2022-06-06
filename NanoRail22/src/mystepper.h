#ifndef NR22mystepper_h
#define NR22mystepper_h

#include <Arduino.h>
#include <pins_arduino.h>
#include "config.h"

uint16_t StepperStepsPerRev = 200; // steps per revoultion (360/1.8)
float StepperSpeed = 0;            // -/+ left/right, revolutions per second

uint32_t StepperStepTO = 0;

#define STEPPER_ON digitalWrite(pin_ENABLE, 0)
#define STEPPER_OFF digitalWrite(pin_ENABLE, 1)
#define STEPPER_INIT_EN pinMode(pin_ENABLE, OUTPUT)

#define STEPPER_DIR_PLUS digitalWrite(pin_X_DIR, 1)
#define STEPPER_DIR_MINUS digitalWrite(pin_X_DIR, 0)
#define STEPPER_INIT_DIR pinMode(pin_X_DIR, OUTPUT)

#define STEPPER_STEP_ON digitalWrite(pin_X_STEP, 1)
#define STEPPER_STEP_OFF digitalWrite(pin_X_STEP, 0)
#define STEPPER_INIT_STEP pinMode(pin_X_STEP, OUTPUT)

// main processing loop
void StepperLoop()
{
    static bool stepstate = 0;

    if ((micros() < StepperStepTO) || (StepperSpeed == 0))
    {
        return;
    }

    if (StepperSpeed > 0)
    {
        STEPPER_DIR_PLUS;
    }
    else if (StepperSpeed < 0)
    {
        STEPPER_DIR_MINUS;
    }

    // makes step
    stepstate = !stepstate;

    float td;
    if (stepstate)
    {
        STEPPER_STEP_ON;
        td = 300;
    }
    else
    {
        STEPPER_STEP_OFF;
        // calc next step
        td = abs(StepperSpeed) * StepperStepsPerRev;
        td = 1000000.0 / td;
        if (td < 300)
            td = 300;
    }

    StepperStepTO = micros() + td;
}

#endif