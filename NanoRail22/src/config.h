#ifndef NR22config_h
#define NR22config_h
#include <Arduino.h>
#include <pins_arduino.h>
#include <AccelStepper.h>

// Simpli use of keyes cnc shield for arduino nano v3
// Shield with 3 stepper drivers A4988


/*
                    Keyestudio CNC Shield V4.0
https://www.keyestudio.com/products/keyestudio-cnc-shield-v40-board-for-arduino-nano
*/

#define pin_unused 0xFF

#define pin_X_STEP  2
#define pin_Y_STEP  3
#define pin_Z_STEP  4
#define pin_X_DIR   5
#define pin_Y_DIR   6
#define pin_Z_DIR   7

#define pin_ENABLE  8  //common for 3 drivers

#define pin_X_LIMIT 9  //common X+ and X-
#define pin_Y_LIMIT 10 //common + -
#define pin_Z_LIMIT 11 //common + -

#define pin_key_CoolEN  A3
#define pin_key_Resume  A2
#define pin_key_Hold    A1
#define pin_key_Abort   A0

#define pin_HeadA7  A7  //  3pin header GND-5V-A7, near DC
#define pin_HeadA6  A6  //  3pin header GND-5V-A6, near DC

#define limitActive 0   //pin value when limit is active
#define keyActive   0   //pin value consider as activeate by user

const uint16_t STEPPER_TIMEOUTms = 30 * 1000;    // 60sek * 1000millis


#endif