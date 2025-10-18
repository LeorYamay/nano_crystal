#include "effects.h"
#include <FastLED.h>
#include <EEPROM.h>

// local state used by effects
static int coordRow = 0;
static int coordCol = 0;
static int vec1 = 1;
static int vec2 = 1;
// accumulated FPS delta requested by effects (can be fractional)
static double requested_fps_delta = 0.0;

void effects_init()
{
  requested_fps_delta = 0.0;
}

double effects_get_and_clear_fps_delta()
{
  double v = requested_fps_delta;
  requested_fps_delta = 0.0;
  return v;
}

uint8_t wrap(uint8_t num, uint8_t limit)
{
  if (num >= limit)
  {
    num = num % limit;
  }
  return num;
}

void UpdateLedHeat(int row, int col)
{
  ledpanel_update_color_from_heat(row, col);
}

void CoolAll(double max, double min)
{
  const int COOLING = 100;
  for (int row = 0; row < ledHeight; row++)
  {
    for (int col = 0; col < numColumns; col++)
    {
      uint8_t newh = qsub8(ledpanel_get_heat(row, col), random8((int)(COOLING * min), (int)(COOLING * max)));
      ledpanel_set_heat(row, col, newh);
      ledpanel_update_color_from_heat(row, col);
    }
  }
}

void Spread(double factor)
{
  // delegate to led_panel spread helper using current coord
  ledpanel_spread_from(coordRow, coordCol, factor);
}

void SpreadHeight(double factor)
{
  // delegate to led_panel vertical spread helper using current coord
  ledpanel_spread_height_from(coordRow, coordCol, factor);
}

void Random()
{
  int dir = random8(0, 30);
  switch (dir)
  {
  case 0:
    vec1 = -1;
    break;
  case 1:
    vec1 = 0;
    break;
  case 2:
    vec2 = -1;
    break;
  case 3:
    vec1 = 1;
    break;
  case 4:
    vec2 = 1;
    break;
  case 5:
    vec2 = 0;
    break;

  default:
    if (vec1 == vec2 and vec1 == 0)
    {
      vec1 = random8(-1, 1);
      vec2 = random8(-1, 1);
    }
    break;
  }
  ledpanel_add_heat(coordRow, coordCol, random8(160, 255));
  Spread(.2);
  if ((coordRow + vec1 == 0) or (coordRow + vec1 == ledHeight - 1))
  {
    vec1 = -vec1;
  }
  coordRow = wrap(coordRow + vec1, ledHeight);
  coordCol = wrap(coordCol + vec2, numColumns);
  CoolAll(0.5, 0.1);
}

void Fire(bool flip)
{

  for (int i = 0; i < numColumns; i++)
  {
    if (flip)
    {
      // Ensure k+2 stays within bounds: valid indices are 0..ledHeight-1
      // so k must be <= ledHeight-3. Previously the loop allowed k such that
      // k+2 == ledHeight which is out-of-bounds and invokes UB.
      for (int k = 2; k <= ledHeight - 3; k++)
      {
        uint8_t v = (ledpanel_get_heat(k + 1, i) + ledpanel_get_heat(k + 2, i) + ledpanel_get_heat(k + 2, i)) / 3;
        ledpanel_set_heat(k, i, v);
        ledpanel_update_color_from_heat(k, i);
      }
    }
    else
    {
      for (int k = ledHeight - 1; k >= 2; k--)
      {
        uint8_t v = (ledpanel_get_heat(k - 1, i) + ledpanel_get_heat(k - 2, i) + ledpanel_get_heat(k - 2, i)) / 3;
        ledpanel_set_heat(k, i, v);
        ledpanel_update_color_from_heat(k, i);
      }
    }
    if (random8() < SPARKING)
    {
      int y;
      if (flip)
      {
        y = random8(ledHeight - 4, ledHeight);
      }
      else
      {
        y = random8(3);
      }
      ledpanel_add_heat(y, i, random8(160, 255));
      ledpanel_update_color_from_heat(y, i);
    }
  }
  CoolAll(0.6, 0.2);
}

void Floating()
{
  int dir = random8(0, 12);
  switch (dir)
  {
  case 0 ... 2:
    vec1 = -1;
    break;
  case 3 ... 5:
    vec1 = 1;
    break;
  case 6:
    vec1 = 0;
    break;
  case 7:
    // adjust frame rate safely from effects - clamp handled by caller
    // Caller (Nano_crystal) maintains FRAMES_PER_SECOND
    // but we can nudge it here if it's accessible; for now leave it alone.
    break;
  default:
    break;
  }
  for (coordCol = 0; coordCol < numColumns; coordCol++)
  {
    ledpanel_add_heat(coordRow, coordCol, random8(180, 255));
    SpreadHeight(0.2);
  }
  CoolAll(0.3, 0);
  if ((coordRow + vec1 == 0) or (coordRow + vec1 == ledHeight - 1))
  {
    vec1 = -vec1;
  }
  coordRow = wrap(coordRow + vec1, ledHeight);
}

void RandomizeTime()
{
  int change = random8(0, 30);
  switch (change)
  {
  case 0:
    requested_fps_delta -= 0.5;
    break;
  case 1:
    requested_fps_delta -= 1.0;
    break;
  case 2 ... 4:
    requested_fps_delta += 0.5;
    break;
  default:
    break;
  }
}
