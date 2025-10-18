#include "led_panel.h"
#include "config.h"
#include <FastLED.h>

// ownership of LED and heat storage lives here
CRGB leds[NUM_LEDS];
static uint8_t heatpan[ledHeight][numColumns];
static CRGBPalette16 panelPalette;

void ledpanel_init()
{
  panelPalette = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::OrangeRed, CRGB::Orange);
  memset(heatpan, 0, sizeof(heatpan));
  FastLED.clear(true);
}

void ledpanel_clear()
{
  memset(heatpan, 0, sizeof(heatpan));
  FastLED.clear(true);
}

int ledpanel_rows()
{
  return ledHeight;
}

int ledpanel_cols()
{
  return numColumns;
}

void ledpanel_set_palette(const CRGBPalette16 &pal)
{
  panelPalette = pal;
}

uint8_t ledpanel_get_heat(int row, int col)
{
  return heatpan[row][col];
}

void ledpanel_set_heat(int row, int col, uint8_t value)
{
  heatpan[row][col] = value;
}

void ledpanel_add_heat(int row, int col, uint8_t delta)
{
  heatpan[row][col] = qadd8(heatpan[row][col], delta);
}

void ledpanel_update_color_from_heat(int row, int col)
{
  uint8_t colorindex = scale8(heatpan[row][col], 240);
  CRGB color = ColorFromPalette(panelPalette, colorindex);
  // mapping same as original posleds(row,col)
  bool even = (col & 1) == 0;
  int index = 0;
  if (even)
  {
    index = row + ledHeight * col;
  }
  else
  {
    index = ledHeight * (col + 1) - row - 1;
  }
  if (index >= 0 && index < NUM_LEDS)
  {
    leds[index] = color;
  }
}

void ledpanel_set_color(int row, int col, const CRGB &color)
{
  bool even = (col & 1) == 0;
  int index = 0;
  if (even)
  {
    index = row + ledHeight * col;
  }
  else
  {
    index = ledHeight * (col + 1) - row - 1;
  }
  if (index < 0)
    index = 0;
  if (index >= NUM_LEDS)
    index = NUM_LEDS - 1;
  leds[index] = color;
}

void ledpanel_cool_all(double maxFactor, double minFactor)
{
  const int COOLING = 100;
  for (int row = 0; row < ledpanel_rows(); row++)
  {
    for (int col = 0; col < ledpanel_cols(); col++)
    {
      heatpan[row][col] = qsub8(heatpan[row][col], random8((int)(COOLING * minFactor), (int)(COOLING * maxFactor)));
      ledpanel_update_color_from_heat(row, col);
    }
  }
}

void ledpanel_spread_from(int row, int col, double factor)
{
  for (int i = -1; i <= 1; i++)
  {
    for (int j = -1; j <= 1; j++)
    {
      if (i == 0 && j == 0)
        continue;
      int r = (row + i + ledpanel_rows()) % ledpanel_rows();
      int c = (col + j + ledpanel_cols()) % ledpanel_cols();
      uint8_t v = (heatpan[r][c] + (uint8_t)(heatpan[row][col] * factor)) / (1 + (int)(factor));
      if (v >= heatpan[row][col])
        v = heatpan[row][col];
      heatpan[r][c] = v;
      ledpanel_update_color_from_heat(r, c);
    }
  }
}

void ledpanel_spread_height_from(int row, int col, double factor)
{
  for (int i = -1; i <= 1; i++)
  {
    if (i == 0)
      continue;
    int r = (row + i + ledpanel_rows()) % ledpanel_rows();
    uint8_t v = heatpan[r][col] + (uint8_t)(heatpan[row][col] * factor);
    if (v > heatpan[row][col])
      v = heatpan[row][col];
    heatpan[r][col] = v;
    ledpanel_update_color_from_heat(r, col);
  }
}

CRGB &ledpanel_at(int row, int col)
{
  bool even = (col & 1) == 0;
  int index = 0;
  if (even)
  {
    index = row + ledHeight * col;
  }
  else
  {
    index = ledHeight * (col + 1) - row - 1;
  }
  if (index < 0)
    index = 0;
  if (index >= NUM_LEDS)
    index = NUM_LEDS - 1;
  return leds[index];
}
