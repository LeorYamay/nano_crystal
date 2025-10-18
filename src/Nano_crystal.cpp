#include "Nano_crystal.h"
#include <FastLED.h>
#include <SoftwareSerial.h>
#include "config.h"

// Globals
bool off = false;

// pins, chipset, geometry, and defaults are in config.h
bool nextSwitch = false;
int setNum = -1;
int prognum = 3;
bool prevSwitch = false;
int colorSchemeNum = 1;

int BRIGHTNESS = DEFAULT_BRIGHTNESS; // runtime brightness (can be changed)
double FRAMES_PER_SECOND = Base_FRAMES_PER_SECOND; // runtime FPS

// bool gReverseDirection = false;

// numColumns, ledHeight, NUM_LEDS are in config.h

// coordinates of current position

// coord and vec state are owned by effects module now

#include "led_panel.h"

// led storage, heatpan and palette now belong to led_panel module

// Forward declarations
void Set();
void PalletSet();
void RunLed();
void OffAction();
void SwitchOff();
void MemUpdate();
int SwitchUp(int value, bool up);

#include "effects.h"

void setup()
{
  Serial.begin(9600); // USB cable
  Serial.println("------setup start------");
  off = false;
  pinMode(POWER_ON, OUTPUT);
  pinMode(POWER_SWITCH, INPUT_PULLUP);
  pinMode(PREV, INPUT_PULLUP);
  pinMode(NEXT, INPUT_PULLUP);
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
  // PalletSwap();
  // ProgramSwap();
  Set();
  RandomizeTime();
  // apply any FPS delta requested by effects
  double fps_delta = effects_get_and_clear_fps_delta();
  FRAMES_PER_SECOND += fps_delta;
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
void Set()
{
  if (!digitalRead(NEXT))
  {
    if (!nextSwitch)
    {
      nextSwitch = true;
      setNum = SwitchUp(setNum, true);
    }
  }
  else
  {
    nextSwitch = false;
  }
  if (!digitalRead(PREV))
  {
    if (!prevSwitch)
    {
      prevSwitch = true;
      setNum = SwitchUp(setNum, false);
    }
  }
  else
  {
    prevSwitch = false;
  }
  switch (setNum)
  {
  case -1:
    prognum = 3;
    colorSchemeNum = 1;
    break;
  case 0:
    prognum = 3;
    colorSchemeNum = 2;
    break;
  case 1:
    prognum = 1;
    colorSchemeNum = 2;
    break;
  default:
    prognum = 0;
    colorSchemeNum = 1;
    setNum = 0;
    break;
  }
}

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
  default:
    break;
  }
}

void PalletSet()
{
  switch (colorSchemeNum)
  {
  case 0:
    ledpanel_set_palette(CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White));
    break;
  case 1:
    ledpanel_set_palette(CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::OrangeRed, CRGB::Orange));
    break;
  case 2:
    ledpanel_set_palette(CRGBPalette16(CRGB::Black, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::GreenYellow));
    break;
  case 3:
    ledpanel_set_palette(CRGBPalette16(CRGB::Black, CRGB::Purple, CRGB::Purple, CRGB::MediumPurple));
    break;
  default:
    break;
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
