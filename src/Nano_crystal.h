#ifndef NANO_CRYSTAL_H
#define NANO_CRYSTAL_H

#include <Arduino.h>
#include <FastLED.h>
#include <EEPROM.h>

// Core Arduino entry points (kept for clarity)
void setup();
void loop();

// Interrupt handler declared so it can be referenced externally if needed
void SwitchOff();

#endif // NANO_CRYSTAL_H
