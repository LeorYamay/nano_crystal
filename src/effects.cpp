#include "effects.h"
#include <FastLED.h>
#include <EEPROM.h>

// Access runtime FPS globals defined in Nano_crystal.cpp so effects can
// bias timing changes relative to the configured base FPS.
extern double FRAMES_PER_SECOND;

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
  // Determine the current effective rate (what will be after pending deltas)
  double effective = FRAMES_PER_SECOND + requested_fps_delta;
  double base = Base_FRAMES_PER_SECOND;

  // Relative difference from base (signed). e.g. 0.5 => 50% above base
  double rel = 0.0;
  if (base > 0.0)
    rel = (effective - base) / base;
  double absrel = fabs(rel);

  // Non-linear bias: cube the distance so only larger deviations strongly
  // influence a pull back toward the base. Clamp to [0,1] for stability.
  if (absrel > 1.0)
    absrel = 1.0;
  double p_toward = pow(absrel, 3.0);

  // Give a small baseline chance to nudge even when close to base and
  // scale the strength slightly so behavior is gentle.
  p_toward = p_toward * 0.9 + 0.05; // now in approx [0.05, 0.95]

  // Hard cap: if at or above 2x base, always move back toward base
  if (effective >= 2.0 * base)
  {
    requested_fps_delta -= 1;
    return;
  }

  // Random roll to decide whether to move toward base (probability p_toward)
  int roll = random8(0, 100); // 0..99
  if (roll < (int)(p_toward * 100.0))
  {
    // Move toward the base
    if (effective > base)
      requested_fps_delta -= 1;
    else if (effective < base)
      requested_fps_delta += 1;
  }
  else
  {
    // Small chance to move away from base (keeps animation lively).
    // Make this much less likely than moving toward the base.
    int awayRoll = random8(0, 100);
    if (awayRoll < 10)
    {
      if (effective > base)
        requested_fps_delta += 1;
      else if (effective < base)
        requested_fps_delta -= 1;
    }
  }
}

// Spiral effect implementation ------------------------------------------------
// 8x8 bitmap frames encoded as 64-bit little-endian rows (LSB is column 0)
const uint64_t IMAGES[] = {
  0x0006090000906000,
  0x000e100000087000,
  0x0c10200000040830,
  0x2040402004020204,
  0x0040404422020200,
  0x0000848241210000
};
const int IMAGES_LEN = sizeof(IMAGES) / sizeof(IMAGES[0]);

static void SpiralImpl(bool mirrored)
{
  static int frame = 0;
  static int ticks = 0;          // advance frame every N calls
  const int TICKS_PER_FRAME = 4; // tweak this to slow/fast animation
  // requested_fps_delta = 1;
  // cool the whole panel mildly each call
  ledpanel_cool_all(0.35, 0.05);
  if (ticks <= 1)
  {
    // heat bits from the current frame
    uint64_t bits = IMAGES[frame % IMAGES_LEN];
    for (int r = 0; r < ledHeight; ++r)
    {
      for (int c = 0; c < numColumns; ++c)
      {
        int bitIndex = r * numColumns + c; // row-major bit index
        bool set = (bits >> bitIndex) & 1ULL;
        if (set)
        {
          if (mirrored)
          {
            int mr = ledHeight - 1 - r;
            // int mc = numColumns - 1 - c;
            ledpanel_add_heat(mr, c, random8(70, 160));
            ledpanel_update_color_from_heat(mr, c);
          }
          else
          {
            ledpanel_add_heat(r, c, random8(70, 160));
            ledpanel_update_color_from_heat(r, c);
          }
        }
      }
    }
  }
  // advance frame after ticks
  ticks++;
  if (ticks >= TICKS_PER_FRAME)
  {
    ticks = 0;
    frame = (frame + 1) % IMAGES_LEN;
  }
  RandomizeTime();
}

void Spiral()
{
  SpiralImpl(false);
}

void SpiralMirrored()
{
  SpiralImpl(true);
}

// Fire wrappers to provide no-arg functions for program array
void FireOff()
{
  Fire(false);
}

void FireOn()
{
  Fire(true);
}

// Pulses effect ------------------------------------------------------------
// Uses the provided 8x8 frames encoded as 64-bit rows
const uint64_t PULSES_IMAGES[] = {
  0x0000000000000000ULL,
  0x0000001818000000ULL,
  0x0000182424180000ULL,
  0x0018244242241800ULL,
  0x3c4281818181423cULL,
};
const int PULSES_LEN = sizeof(PULSES_IMAGES) / sizeof(PULSES_IMAGES[0]);

// Playback sequence: 0,1,2,3,4,2,3,0
// Frame amplitudes (0 = idle, 4 = full pulse)
static const int PULSES_SEQ[] = {
    0, 2, 4, 1, 0,    // First beat
    2, 3, 1, 0       // Second smaller beat
};

static const int PULSES_SEQ_LEN = sizeof(PULSES_SEQ) / sizeof(PULSES_SEQ[0]);

// Duration per step in ticks
static int PULSES_TICKS[PULSES_SEQ_LEN] = {
  3,  // 0 → idle base
  3,  // rise
  4,  // peak
  2,  // fall (don't drop directly to 0)
    7, // short pause before second beat

    3,  // rise
    4,  // smaller peak
    3,  // fall
    28  // long rest before repeating
};


static void PulsesImpl()
{
  static int seqIndex = 0;    // which step in the sequence
  static int tickCounter = 0; // ticks spent on current step
  int img = PULSES_SEQ[seqIndex];

  // Render the frame only on the first tick of the step (add heat once)
  if (tickCounter == 0)
  {
    uint64_t bits = PULSES_IMAGES[img % PULSES_LEN];
    for (int r = 0; r < ledHeight; ++r)
    {
      for (int c = 0; c < numColumns; ++c)
      {
        int bitIndex = r * numColumns + c;
        bool set = (bits >> bitIndex) & 1ULL;
        if (set)
        {
          ledpanel_add_heat(r, c, random8(120, 220));
          ledpanel_update_color_from_heat(r, c);
        }
      }
    }
  }

  // Gentle cooling every call (including the first after adding heat)
  ledpanel_cool_all(0.3, 0.05);

  // advance ticks for this sequence step
  tickCounter++;
  if (tickCounter >= PULSES_TICKS[seqIndex])
  {
    tickCounter = 0;
    seqIndex++;
    if (seqIndex >= PULSES_SEQ_LEN)
      seqIndex = 0;
  }

  RandomizeTime();
}

void Pulses()
{
  PulsesImpl();
}
