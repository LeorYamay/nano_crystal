#ifndef LED_PANEL_H
#define LED_PANEL_H

#include "config.h"
#include <FastLED.h>

// Owned storage (defined in led_panel.cpp)
extern CRGB leds[NUM_LEDS];

// initialize/clear
void ledpanel_init();
void ledpanel_clear();

// geometry
int ledpanel_rows();
int ledpanel_cols();

// palette
void ledpanel_set_palette(const CRGBPalette16 &pal);

// heat accessors
uint8_t ledpanel_get_heat(int row, int col);
void ledpanel_set_heat(int row, int col, uint8_t value);
void ledpanel_add_heat(int row, int col, uint8_t delta);

// color update (compute color from heat and write to leds)
void ledpanel_update_color_from_heat(int row, int col);

// direct LED reference by (row,col)
CRGB &ledpanel_at(int row, int col);

// spread helpers (neighbor propagation)
void ledpanel_spread_from(int row, int col, double factor);
void ledpanel_spread_height_from(int row, int col, double factor);

// direct set of color
void ledpanel_set_color(int row, int col, const CRGB &color);

// cool all helper
void ledpanel_cool_all(double maxFactor, double minFactor);

#endif // LED_PANEL_H
