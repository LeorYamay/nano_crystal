#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// Hardware pins and LED chipset
#define ONBOARD_LED 13
#define POWER_ON 7
#define POWER_SWITCH 2
#define PROGRAM_BUTTON 6
#define COLOR_BUTTON 5
#define LED_PIN A1
#define SMOKE_RELAY 3
#define COLOR_ORDER GRB
#define CHIPSET WS2813

// Display geometry
static const int numColumns = 8;
static const int ledHeight = 8;
static const int NUM_LEDS = numColumns * ledHeight;

// Brightness & timing defaults
static const int DEFAULT_BRIGHTNESS = 255;
// Base frames per second. Increase by 3.5x from the previous value.
// Use a double so fractional FPS is supported.
static const double Base_FRAMES_PER_SECOND = 9.0 * 3.5; // 31.5

// Fire effect tuning
static const uint8_t SPARKING = 50; // chance out of 255

// Smoke relay timing constants (in milliseconds)
static const unsigned long SMOKE_RELAY_HIGH_TIME = 3000;  // 3 seconds
static const unsigned long SMOKE_RELAY_LOW_TIME = SMOKE_RELAY_HIGH_TIME*2.5;   
static const int SMOKE_RELAY_CYCLES = 5;                   // repeat 5 times

// Centralized program count (update when adding/removing programs)
static const int MAX_PROGRAMS = 5;

#endif // CONFIG_H

