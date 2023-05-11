#include <FastLED.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

bool off = false;
int vec1 = 1;
int vec2 = 1;

#pragma region arduinopins
#define ONBOARD_LED 13
#define POWER_ON 7
#define POWER_SWITCH 2
#define COLOR 6
#define PROG 5
#define LED_PIN A1
#define COLOR_ORDER GRB
#define CHIPSET WS2813
#pragma endregion arduinopins
bool progSwitch = false;
int prognum = 3;
bool colorSwitch = false;
int colorSchemeNum = 1;

int BRIGHTNESS = 255;
double FRAMES_PER_SECOND = 9;

// const int Base_BRIGHTNESS = 200;
const int Base_FRAMES_PER_SECOND = 9;

// bool gReverseDirection = false;

const int numColumns = 8;
const int ledHeight = 8;

// coordinates of current position

int coordRow = 0;
int coordCol = 0;

const int NUM_LEDS = numColumns * ledHeight;

CRGB leds[NUM_LEDS];
CRGB &posleds(int row, int col)
{
  // maps single panel to leds
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

  if (index > NUM_LEDS)
  {
    index = NUM_LEDS;
  }
  if (index < 0)
  {
    index = 0;
  };
  return leds[index];
}

CRGBPalette16 gPal;

// 2d Array of temperature readings at each simulation cell
// [row] [column]
static uint8_t heatpan[ledHeight][numColumns];

void setup()
{
  Serial.begin(9600); // USB cable
  Serial.println("------setup start------");
  off = false;
  pinMode(POWER_ON, OUTPUT);
  pinMode(POWER_SWITCH, INPUT_PULLUP);
  pinMode(COLOR, INPUT_PULLUP);
  pinMode(PROG, INPUT_PULLUP);
  // pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(POWER_ON, HIGH);
  // digitalWrite(ONBOARD_LED, HIGH);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  attachInterrupt(0, SwitchOff, CHANGE);
  // colorSchemeNum = EEPROM.read(0);
  // prognum = EEPROM.read(1);
  delay(1000);
  Serial.println("------setup done------");
}

void loop()
{
  random16_add_entropy(random());
  PalletSwap();
  ProgramSwap();
  RandomizeTime();
  if (!off)
  {
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

void ProgramSwap()
{
  if (!digitalRead(PROG))
  {
    if (!progSwitch)
    {
      Serial.print("Program change: ");
      progSwitch = true;
      prognum += 1;
      FastLED.clear(true);
      FastLED.show(); // display this frame
      FastLED.delay(1000 / FRAMES_PER_SECOND);
      switch (prognum)
      {
      case 0:
        Serial.println("Fire");
        break;
      case 1:
        Serial.println("Random");
        break;
      case 2:
        Serial.println("Floating");
        break;
      case 3:
        Serial.println("Rain");
        break;

      default:
        prognum = 0;
        break;
      }
    }
  }
  else
  {
    progSwitch = false;
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
void PalletSwap()
{
  if (!digitalRead(COLOR))
  {
    if (!colorSwitch)
    {
      Serial.print("colorscheme change: ");
      colorSwitch = true;
      colorSchemeNum += 1;
      switch (colorSchemeNum)
      {
      case 0:
        Serial.println("BLUE");
        break;
      case 1:
        Serial.println("RED");
        break;
      case 2:
        Serial.println("GREEN");
        break;
      case 3:
        Serial.println("Purple");
        break;
      default:
        colorSchemeNum = 0;
        break;
      }
      PalletSet();
      FastLED.clear(true);
      FastLED.show(); // display this frame
      FastLED.delay(1000 / FRAMES_PER_SECOND);
    }
  }
  else
  {
    colorSwitch = false;
  }
  PalletSet();
}
void PalletSet()
{
  switch (colorSchemeNum)
  {
  case 0:
    gPal = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
    break;
  case 1:
    gPal = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::OrangeRed, CRGB::TempOrange);
    break;
  case 2:
    gPal = CRGBPalette16(CRGB::Black, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::GreenYellow);
    break;
  case 3:
    gPal = CRGBPalette16(CRGB::Black, CRGB::Purple, CRGB::Purple, CRGB::MediumPurple);
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

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 50

void CoolAll(double max, double min)
{
  const int COOLING = 100;
  for (int row = 0; row < ledHeight; row++)
  {
    for (int col = 0; col < numColumns; col++)
    {
      heatpan[row][col] = qsub8(heatpan[row][col], random8((COOLING * min), (COOLING * max)));
      UpdateLedHeat(row, col);
    }
  }
}
uint8_t wrap(uint8_t num, uint8_t limit)
{
  if (num >= limit)
  {
    num = num % limit;
  }
  return num;
}
void Spread(double factor)
{
  int row = 0;
  int col = 0;
  for (int i = -1; i <= 1; i++)
  {
    for (int j = -1; j <= 1; j++)
    {
      if ((i == 0) and (j == 0))
      {
      }
      else
      {
        row = wrap(coordRow + i, ledHeight);
        col = wrap(coordCol + j, numColumns);
        heatpan[row][col] = (heatpan[row][col] + heatpan[coordRow][coordCol] * factor) / (1 + factor);
        if (heatpan[row][col] >= heatpan[coordRow][coordCol])
        {
          heatpan[row][col] = heatpan[coordRow][coordCol];
        }
        UpdateLedHeat(row, col);
      }
    }
  }
}
void SpreadHeight(double factor)
{
  int row = 0;
  for (int i = -1; i <= 1; i++)
  {

    if (i == 0)
    {
    }
    else
    {
      row = wrap(coordRow + i, ledHeight);
      heatpan[row][coordCol] = (heatpan[row][coordCol] + heatpan[coordRow][coordCol] * factor);
      if (heatpan[row][coordCol] > heatpan[coordRow][coordCol])
      {
        heatpan[row][coordCol] = heatpan[coordRow][coordCol];
      }
      UpdateLedHeat(row, coordCol);
    }
  }
}
void UpdateLedHeat(int row, int col)
{
  uint8_t colorindex = scale8(heatpan[row][col], 240);
  CRGB color = ColorFromPalette(gPal, colorindex);
  posleds(row, col) = color;
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
  heatpan[coordRow][coordCol] = qadd8(heatpan[coordRow][coordCol], random8(160, 255));
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
      for (int k = 2; k < ledHeight - 1; k++)
      {
        heatpan[k][i] = (heatpan[k + 1][i] + heatpan[k + 2][i] + heatpan[k + 2][i]) / 3;
        UpdateLedHeat(k, i);
      }
    }
    else
    {
      for (int k = ledHeight - 1; k >= 2; k--)
      {
        heatpan[k][i] = (heatpan[k - 1][i] + heatpan[k - 2][i] + heatpan[k - 2][i]) / 3;
        UpdateLedHeat(k, i);
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
      heatpan[y][i] = qadd8(heatpan[y][i], random8(160, 255));
      UpdateLedHeat(y, i);
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
    FRAMES_PER_SECOND--;
  default:
    break;
  }
  for (coordCol = 0; coordCol < numColumns; coordCol++)
  {
    heatpan[coordRow][coordCol] = qadd8(heatpan[coordRow][coordCol], random8(180, 255));
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
    FRAMES_PER_SECOND -= 0.5;
    break;
  case 1:
    FRAMES_PER_SECOND -= 1;
    break;
  case 2 ... 4:
    FRAMES_PER_SECOND += 0.5;
    break;
  default:
    break;
  }
  if (FRAMES_PER_SECOND < Base_FRAMES_PER_SECOND - 3)
  {
    FRAMES_PER_SECOND = Base_FRAMES_PER_SECOND - 3;
  }
  if (FRAMES_PER_SECOND > Base_FRAMES_PER_SECOND + 2)
  {
    FRAMES_PER_SECOND = Base_FRAMES_PER_SECOND + 2;
  }
}