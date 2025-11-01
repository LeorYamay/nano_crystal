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
double FRAMES_PER_SECOND = Base_FRAMES_PER_SECOND*3.5; // runtime FPS

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

void setup()
{
  Serial.begin(9600); // USB cable
  Serial.println("------setup start------");
  off = false;
  pinMode(POWER_ON, OUTPUT);
  pinMode(POWER_SWITCH, INPUT_PULLUP);
  pinMode(PROGRAM_BUTTON, INPUT_PULLUP);
  pinMode(COLOR_BUTTON, INPUT_PULLUP);
  // pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(POWER_ON, HIGH);
  // digitalWrite(ONBOARD_LED, HIGH);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  attachInterrupt(0, SwitchOff, CHANGE);
  // colorSchemeNum = EEPROM.read(0);
  // prognum = EEPROM.read(1);
  effects_init();
  delay(1000);
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
  switch (prognum)
  {
  case 0:
    Fire(false);
    break;
  case 1:
    Random();
    break;
  case 2:
    Floating();
    break;
  case 3:
    Fire(true);
    break;
  case 4:
    Spiral();
    break;
  case 5:
    SpiralMirrored();
    break;
  default:
    break;
  }
}

void PalletSet()
{
  // Use the centralized palette array. Clamp colorSchemeNum to valid range.
  if (colorSchemeNum < 0)
    colorSchemeNum = 0;
  if (colorSchemeNum >= (int)PALETTES_COUNT)
    colorSchemeNum = PALETTES_COUNT - 1;
  ledpanel_set_palette(PALETTES[colorSchemeNum]);
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
