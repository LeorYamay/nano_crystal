#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// Hardware pins and LED chipset
#define ONBOARD_LED 13
#define POWER_ON 7
#define POWER_SWITCH 2
#define PREV 6
#define NEXT 5
#define LED_PIN A1
#define COLOR_ORDER GRB
#define CHIPSET WS2813

// Display geometry
static const int numColumns = 8;
static const int ledHeight = 8;
static const int NUM_LEDS = numColumns * ledHeight;

// Brightness & timing defaults
static const int DEFAULT_BRIGHTNESS = 255;
static const int Base_FRAMES_PER_SECOND = 9;

// Fire effect tuning
static const uint8_t SPARKING = 50; // chance out of 255

#endif // CONFIG_H

