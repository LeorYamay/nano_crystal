#include "Nano_crystal.h"
#include <FastLED.h>
#include <SoftwareSerial.h>
#include "config.h"

// Globals
bool off = false;

// pins, chipset, geometry, and defaults are in config.h
bool nextSwitch = false;
int setNum = -1;
int prognum = 4;
bool prevSwitch = false;
int colorSchemeNum = 1;

int BRIGHTNESS = DEFAULT_BRIGHTNESS; // runtime brightness (can be changed)
double FRAMES_PER_SECOND = Base_FRAMES_PER_SECOND; // runtime FPS — start at base

// Smoke relay control state
static unsigned long smoke_relay_start_time = 0;   // when current state began
static bool smoke_relay_is_on = true;               // true = HIGH, false = LOW
static int smoke_relay_cycle_count = 0;             // current cycle (0 to SMOKE_RELAY_CYCLES-1)
static bool smoke_relay_active = true;              // whether the relay cycle is running

// bool gReverseDirection = false;

// numColumns, ledHeight, NUM_LEDS are in config.h

// coordinates of current position

// coord and vec state are owned by effects module now

#include "led_panel.h"

// led storage, heatpan and palette now belong to led_panel module

// Forward declarations
// void Set();
void PalletSet();
void PalletSwap();
void ProgramSwap();
void RunLed();
void OffAction();
void SwitchOff();
void MemUpdate();
int SwitchUp(int value, bool up);

#include "effects.h"
#include "palettes.h"
#include "programs.h"

void setup()
{
  Serial.begin(9600); // USB cable
  Serial.println("------setup start------");
  off = false;
  pinMode(POWER_ON, OUTPUT);
  pinMode(POWER_SWITCH, INPUT_PULLUP);
  pinMode(PROGRAM_BUTTON, INPUT_PULLUP);
  pinMode(COLOR_BUTTON, INPUT_PULLUP);
  pinMode(SMOKE_RELAY, OUTPUT);
  // pinMode(ONBOARD_LED, OUTPUT);
  delay(1000);
  digitalWrite(SMOKE_RELAY, HIGH); // start relay ON
  smoke_relay_start_time = millis();
  // digitalWrite(POWER_ON, HIGH);
  // digitalWrite(ONBOARD_LED, HIGH);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  // attachInterrupt(0, SwitchOff, CHANGE);
  // colorSchemeNum = EEPROM.read(0);
  // prognum = EEPROM.read(1);
  effects_init();
  // Seed the PRNG with a floating analog pin for entropy and add to
  // the 16-bit random pool. This helps ensure different runs are different.
  int seed = analogRead(A7);
  randomSeed(seed);
  random16_add_entropy((uint16_t)(seed ^ millis()));
  colorSchemeNum = random8(0, (uint8_t)PALETTES_COUNT);
  // colorSchemeNum = 1;
  Serial.print("Initial random seed (A7): ");
  Serial.println(seed);
  Serial.print("Initial random palette number: ");
  Serial.println(colorSchemeNum);
  // // choose among PRG_SPIRAL, PRG_SPIRAL_MIRRORED, PRG_PULSES
  // {
  //   uint8_t choice = random8(0, 3);
  //   switch (choice)
  //   {
  //   case 0:
  //     prognum = PRG_SPIRAL;
  //     break;
  //   case 1:
  //     prognum = PRG_SPIRAL_MIRRORED;
  //     break;
  //   default:
  //     prognum = PRG_PULSES;
  //     break;
  //   }
  //   Serial.println("Initial random program number: " + String(prognum) );
    
  // }
  prognum = PRG_PULSES;
  Serial.println("Initial program number-PRG_PULSES: " + String(prognum) );
  Serial.println("------setup done------");
}

void loop()
{
  random16_add_entropy(random());
  // Comment out preset Set() behavior and restore independent controls
  // Set();
  PalletSwap();
  ProgramSwap();
  RandomizeTime();
  // apply any FPS delta requested by effects
  double fps_delta = effects_get_and_clear_fps_delta();
  FRAMES_PER_SECOND += fps_delta;
  // Enforce a maximum FPS = 2 * the base FPS
  {
    double max_fps = Base_FRAMES_PER_SECOND * 2.0;
    if (FRAMES_PER_SECOND > max_fps)
      FRAMES_PER_SECOND = max_fps;
  }
  // Enforce a minimum FPS = half the base FPS
  {
    double min_fps = Base_FRAMES_PER_SECOND / 2.0;
    if (FRAMES_PER_SECOND < min_fps)
      FRAMES_PER_SECOND = min_fps;
  }
  // Control smoke relay: alternate HIGH/LOW in cycles
  if (smoke_relay_active)
  {
    unsigned long elapsed = millis() - smoke_relay_start_time;
    if (smoke_relay_is_on)
    {
      // Currently HIGH, check if time to switch to LOW
      if (elapsed >= SMOKE_RELAY_HIGH_TIME)
      {
        digitalWrite(SMOKE_RELAY, LOW);
        smoke_relay_is_on = false;
        smoke_relay_start_time = millis();
      }
    }
    else
    {
      // Currently LOW, check if time to switch to HIGH or end cycle
      if (elapsed >= SMOKE_RELAY_LOW_TIME)
      {
        smoke_relay_cycle_count++;
        if (smoke_relay_cycle_count >= SMOKE_RELAY_CYCLES)
        {
          // All cycles complete, turn relay OFF and stop
          digitalWrite(SMOKE_RELAY, LOW);
          smoke_relay_active = false;
          Serial.println("Smoke relay cycles complete.");
        }
        else
        {
          // Start next cycle: turn relay back ON
          digitalWrite(SMOKE_RELAY, HIGH);
          smoke_relay_is_on = true;
          smoke_relay_start_time = millis();
        }
      }
    }
  }
  // PalletSet();
  //   RunLed();
  if (!off)
  {
    PalletSet();
    RunLed();
  }
  else
  {
    OffAction();
  }
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  while (off)
  {
    OffAction();
  }
}

int SwitchUp(int value, bool up)
{
  int temp = value;
  if (up)
  {
    temp++;
  }
  else
  {
    temp--;
  }
  if (abs(temp) > 1)
  {
    return value;
  }
  else
    return temp;
}
// void Set()
// {
//   if (!digitalRead(NEXT))
//   {
//     if (!nextSwitch)
//     {
//       nextSwitch = true;
//       setNum = SwitchUp(setNum, true);
//     }
//   }
//   else
//   {
//     nextSwitch = false;
//   }
//   if (!digitalRead(PREV))
//   {
//     if (!prevSwitch)
//     {
//       prevSwitch = true;
//       setNum = SwitchUp(setNum, false);
//     }
//   }
//   else
//   {
//     prevSwitch = false;
//   }
//   switch (setNum)
//   {
//   case -1:
//     prognum = 3;
//     colorSchemeNum = 1;
//     break;
//   case 0:
//     prognum = 3;
//     colorSchemeNum = 2;
//     break;
//   case 1:
//     prognum = 1;
//     colorSchemeNum = 2;
//     break;
//   default:
//     prognum = 0;
//     colorSchemeNum = 1;
//     setNum = 0;
//     break;
//   }
// }

void RunLed()
{
  // Bound-check prognum and call the program function pointer
  if (prognum < 0)
    prognum = 0;
  if (prognum >= (int)PROGRAMS_COUNT)
    prognum = PROGRAMS_COUNT - 1;
  PROGRAMS[prognum]();
}

void PalletSet()
{
  // Use the centralized palette array. Clamp colorSchemeNum to valid range.
  if (colorSchemeNum < 0)
    colorSchemeNum = 0;
  if (colorSchemeNum >= (int)PALETTES_COUNT)
    colorSchemeNum = PALETTES_COUNT - 1;
  ledpanel_set_palette(PALETTES[colorSchemeNum]);

  // Apply palette-specific brightness modifier (relative to DEFAULT_BRIGHTNESS)
  int mod = 0;
  if (colorSchemeNum >= 0 && colorSchemeNum < (int)PALETTES_COUNT)
  {
    mod = PALETTE_BRIGHTNESS_MOD[colorSchemeNum];
  }
  int newBrightness = DEFAULT_BRIGHTNESS + mod;
  if (newBrightness < 0)
    newBrightness = 0;
  if (newBrightness > 255)
    newBrightness = 255;
  BRIGHTNESS = newBrightness;
  FastLED.setBrightness(BRIGHTNESS);
}
// Advance palette (color) when COLOR_BUTTON is pressed; wrap 0..MAX_PALETTES
void PalletSwap()
{
  static bool colorPressed = false;
  if (!digitalRead(COLOR_BUTTON))
  {
    if (!colorPressed)
    {
      colorPressed = true;
      colorSchemeNum++;
      if (colorSchemeNum >= (int)PALETTES_COUNT)
        colorSchemeNum = 0;
    }
  }
  else
  {
    colorPressed = false;
  }
}

// Advance program when PROGRAM_BUTTON is pressed; wrap 0..MAX_PROGRAMS
void ProgramSwap()
{
  static bool progPressed = false;
  if (!digitalRead(PROGRAM_BUTTON))
  {
    if (!progPressed)
    {
      progPressed = true;
      prognum++;
      if (prognum > MAX_PROGRAMS)
        prognum = 0;
    }
  }
  else
  {
    progPressed = false;
  }
}
void SwitchOff()
{
  if (!digitalRead(10))
  {
    off = true;
    Serial.println("Switch off");
  }
}
void MemUpdate()
{
  EEPROM.update(0, colorSchemeNum);
  EEPROM.update(1, prognum);
}
void OffAction()
{
  // MemUpdate();
  // digitalWrite(ONBOARD_LED, LOW);
  digitalWrite(POWER_ON, LOW);
  FastLED.clear(true);
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  // digitalWrite(ONBOARD_LED, HIGH);
}

// Effect implementations moved to src/effects.cpp
