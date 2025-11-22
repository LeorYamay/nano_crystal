// Central palette definitions for Nano_crystal
#ifndef PALETTES_H
#define PALETTES_H

#include <FastLED.h>

// Define palettes here so other modules can reference them by index.
static const CRGBPalette16 PALETTES[] = {
    CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White),
    CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::OrangeRed, CRGB::Orange),
    CRGBPalette16(CRGB::Black, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::GreenYellow),
    CRGBPalette16(CRGB::Black, CRGB::Purple, CRGB::Purple, CRGB::MediumPurple),
};

// Number of palettes available
#define PALETTES_COUNT (sizeof(PALETTES) / sizeof(PALETTES[0]))

// Per-palette brightness modifiers (added to DEFAULT_BRIGHTNESS)
// index 1 -> 0, others -> -25
static const int PALETTE_BRIGHTNESS_MOD[] = {
    -55, // palette 0
    0,   // palette 1 (no modifier)
    -55, // palette 2
    -55  // palette 3
};

#endif // PALETTES_H
