#include <Arduino.h>
//#include <EEPROM.h>
#include <AccelStepper.h>
#include "config.h"

#define HITlimit(x) (x == limitActive)
#define HITkey(x) (x == keyActive)

#define pinXmin pin_X_LIMIT
#define pinXmax pin_X_LIMIT
#define pinButMinus pin_key_Resume
#define pinButPlus pin_key_Hold
#define pinButMenu pin_key_Abort

#define STEPXL -1    // direction of stepper to reach minlimit
#define STEPXH 1     // direction of stepper to reach maxlimit
#define STEPSpeed 10 // predkosc rozpoczynajaca ruch
#define STEPMAXSPEED  2000 //maksymalna predkosc silnika

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
  StepperX.setPinsInverted(false, false, true); //(bool directionInvert, bool stepInvert, bool enableInvert
  StepperX.enableOutputs();

  StepperX.setAcceleration(STEPMAXSPEED/4);  //
  StepperX.setMaxSpeed(STEPMAXSPEED);
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
    // Limit max
    if (getNewPinState(pinXmax, &ValuePinXmax))
    {
      if (HITlimit(ValuePinXmax))
      {
        Serial.print(F("\r\n...Limit Xmax."));
        shouldStop = true;
      }
    }
    // Limit min
    if (getNewPinState(pinXmin, &ValuePinXmin))
    {
      if (HITlimit(ValuePinXmin))
      {
        Serial.print(F("\r\n...Limit Xmin."));
        shouldStop = true;
      }
    }
    // czy zmieniony stan przyciskow
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
    else if (HITkey(ValuePinButMinus)) // wcisniety przycisk
    {
      carrSPEEDx10 = carrSPEEDx10 + (100 * STEPXL);
      shouldMove = true;
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
    else if (HITkey(ValuePinButPlus)) // wcisniety przycisk
    {
      carrSPEEDx10 = carrSPEEDx10 + (100 * STEPXH);
      shouldMove = true;
    }

    if (getNewPinState(pinButMenu, &ValuePinButMenu))
    {
      if (HITkey(ValuePinButMenu))
      {
        //
      }
    }

    if (carrSPEEDx10 > STEPMAXSPEED*10)
      carrSPEEDx10 = STEPMAXSPEED*10;
    if (carrSPEEDx10 < -STEPMAXSPEED*10)
      carrSPEEDx10 = -STEPMAXSPEED*10;

    if (shouldStop)
    {
      if (!isIdle)
      {
        Serial.print(F("\tStop Motor with speed: "));
        Serial.print(StepperX.speed());
        //StepperX.stop();
        StepperX.setSpeed(0.0);
        isIdle = true;
        idleStepperTimeout = millis() + STEPPER_TIMEOUTms;
        carrSPEEDx10 = 0;
      }
    }
    else if (shouldMove)
    {
        float ns = carrSPEEDx10 / 10;
        if (StepperX.speed() != ns)
        {
          StepperX.setSpeed(carrSPEEDx10 / 10);
          isIdle = false;
          isStepperEnabled = true;
          StepperX.enableOutputs();
          idleStepperTimeout = UINT32_MAX;
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
  /*if (pin < A6)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }*/
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