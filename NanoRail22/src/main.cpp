#include <Arduino.h>
//#include <EEPROM.h>
#include <AccelStepper.h>
#include "config.h"

#define HITlimit(x) (x == limitActive)
#define HITkey(x) (x == keyActive)

#define pinXmin pin_HeadA7
#define pinXmax pin_HeadA6
#define pinButMinus pin_key_Resume
#define pinButPlus pin_key_Hold
#define pinButMenu pin_key_Abort

#define STEPXL -1     // direction of stepper to reach minlimit
#define STEPXH 1      // direction of stepper to reach maxlimit
#define STEPSpeed 250 // max speed for stepper

bool debugPinsState = false; // true anables printing pin value when changing

bool isIdle = true;
bool isStepperEnabled = true;
uint32_t idleStepperTimeout = STEPPER_TIMEOUTms;

int16_t carrSPEEDx10 = 0;

uint32_t TimeoutLED = 0;
bool ValueLED = LOW;

uint32_t TimeoutPins = 0;
uint8_t ValuePinXmin = !limitActive;
uint8_t ValuePinXmax = !limitActive;
uint8_t ValuePinButMinus = !keyActive;
uint8_t ValuePinButPlus = !keyActive;
uint8_t ValuePinButMenu = !keyActive;

AccelStepper StepperX = AccelStepper(AccelStepper::DRIVER, pin_X_STEP, pin_X_DIR);
// some functions
bool getNewPinState(uint8_t pin, uint8_t *state);
// enables pullup and set pin as input
void enablePullUp(uint8_t pin);

// main
void setup()
{
  Serial.begin(9600);

  enablePullUp(pinXmin);
  enablePullUp(pinXmax);

  enablePullUp(pinButMinus);
  enablePullUp(pinButPlus);
  enablePullUp(pinButMenu);

  StepperX.setEnablePin(pin_ENABLE);

  StepperX.setAcceleration(STEPSpeed / 4);
  StepperX.setMaxSpeed(STEPSpeed);
  StepperX.setSpeed(0.0);

  Serial.print(F("\r\n...Start loop."));
}

void loop()
{

  if (isIdle)
  {
    if (isStepperEnabled && (millis() > idleStepperTimeout))
    { // disable drivers
      Serial.print(F("\r\n...Disable drivers."));
      StepperX.disableOutputs();
      isStepperEnabled = false;
      carrSPEEDx10 = 0;
    }
  }

  if (millis() > TimeoutPins)
  {
    bool shouldStop = false;
    bool shouldMove = false;

    if (getNewPinState(pinXmax, &ValuePinXmax))
    {
      if (HITlimit(ValuePinXmax))
      {
        Serial.print(F("\r\n...Limit Xmax."));
        shouldStop = true;
      }
    }

    if (getNewPinState(pinXmin, &ValuePinXmin))
    {
      if (HITlimit(ValuePinXmin))
      {
        Serial.print(F("\r\n...Limit Xmin."));
        shouldStop = true;
      }
    }

    if (getNewPinState(pinButMinus, &ValuePinButMinus))
    {
      if (HITkey(ValuePinButMinus))
      {
        carrSPEEDx10 = STEPSpeed * 10 * STEPXL;
        shouldMove = true;
      }
      else
      {
        shouldStop = true;
      }
    }

    if (getNewPinState(pinButPlus, &ValuePinButPlus))
    {
      if (HITkey(ValuePinButPlus))
      {
        carrSPEEDx10 = STEPSpeed * 10 * STEPXH;
        shouldMove = true;
      }
      else
      {
        shouldStop = true;
      }
    }

    if (getNewPinState(pinButMenu, &ValuePinButMenu))
    {
      if (HITkey(ValuePinButMenu))
      {
        //
      }
    }

    if (shouldStop)
    {
      if (!isIdle)
      {
        StepperX.stop();
        StepperX.setSpeed(0.0);
        isIdle = true;
        idleStepperTimeout = millis() + STEPPER_TIMEOUTms;
        carrSPEEDx10 = 0;
        Serial.print(F("\tStop Motor."));
      }
    }
    else
    {
      if (shouldMove)
      {
        if (StepperX.speed() != (carrSPEEDx10 / 10))
        {
          StepperX.setSpeed(carrSPEEDx10 / 10);
          isIdle = false;
          isStepperEnabled = true;
          idleStepperTimeout = UINT32_MAX;
          Serial.print(F("\r\nStart Motor. With speed: "));
          Serial.print(StepperX.speed());
        }
      }
    }

    TimeoutPins = millis() + 10 + (isIdle * 100);
  } //--------timeout pins

  if (millis() > TimeoutLED)
  {
    ValueLED = !ValueLED;
    digitalWrite(LED_BUILTIN, ValueLED);
    TimeoutLED = millis() + 1000 + (1000 * isIdle) - (900 * ValueLED);
  } //---------timeout LED

  StepperX.runSpeed();
} //   ----------------end loop

void enablePullUp(uint8_t pin)
{
  if (pin < A6)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }
  pinMode(pin, INPUT_PULLUP);
}

bool getNewPinState(uint8_t pin, uint8_t *state)
{
  uint8_t s;
  if (pin > A5) // obejscie problemu z digitalRead na portach A6 A7
  {
    if (analogRead(pin) > 750)
    {
      s = true;
    }
    else
      s = false;
  }
  else
    s = digitalRead(pin);

  if (s != *state)
  {
    if (debugPinsState)
    {
      Serial.print(F("\r\nPin "));
      Serial.print(pin);
      Serial.print(F("\tvalue: "));
      Serial.print(s);
    }
    *state = s;
    return true;
  }
  else
    return false;
}