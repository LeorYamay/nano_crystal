#include <FastLED.h>
#include <SoftwareSerial.h>
bool off = false;

#define ONBOARD_LED 13
#define POWER_ON 7
#define POWER_SWITCH 2
#define COLOR 6
#define PROG 5
bool progSwitch = false;
int prognum = 0;
bool colorSwitch = false;
int colorSchemeNum =0;

#define LED_PIN     A0
#define COLOR_ORDER GRB
#define CHIPSET     WS2813
#define NUM_LEDS    144

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 9
bool gReverseDirection = false;



#define numColumns 6 //3,4,6,8,12
#define ledHeight round(NUM_LEDS/numColumns)

CRGB leds[NUM_LEDS];
CRGB& posleds(int row, int col){
  int index =row+ledHeight*col;
  if (index > NUM_LEDS) {index=144;}
  if (index < 0) {index=0;};
  return leds[index];
}

CRGBPalette16 gPal;

void setup() {
  Serial.begin(9600);//USB cable
  Serial.println("------setup start------");
  off = false;
  pinMode(POWER_ON, OUTPUT);
  pinMode(POWER_SWITCH, INPUT_PULLUP);
  pinMode(COLOR, INPUT_PULLUP);
  pinMode(PROG, INPUT_PULLUP);
  pinMode(ONBOARD_LED, OUTPUT);
  gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  digitalWrite(POWER_ON, HIGH);
  digitalWrite(ONBOARD_LED, HIGH);
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  attachInterrupt(0,SwitchOff,CHANGE);
  delay(1000);
  Serial.println("------setup done------");
}

void loop() {
   random16_add_entropy( random());
  if (!off){
    Fire();
  }else{
    OffAction();
  }
  PalletSwap();
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
    while (off){
    OffAction();
  }
  // Serial.print(digitalRead(COLOR));
  // Serial.print(",");
  // Serial.println(digitalRead(PROG));
}

void PalletSwap()
{
  if (!digitalRead(COLOR) )
  {
    if(!colorSwitch)  {
      Serial.print("colorscheme change: ");
      colorSwitch = true;
      colorSchemeNum += 1;
      switch (colorSchemeNum)
      {
      case 0:
         gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
         Serial.println("BLUE");
        break;
      case 1:
        gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
        Serial.println("RED");
      break;
      case 2:
        gPal = CRGBPalette16( CRGB::Black, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::GreenYellow);
         Serial.println("GREEN");
      break;
      default:
      colorSchemeNum=-1;
        break;
      }

    }
  }
  else{
    colorSwitch = false;
  }
}

void SwitchOff()
{
  if (!digitalRead(10)) {
    off = true;
    Serial.println("Switch off");
  }
  // while (off){
  //   OffAction();
  // }
}

void OffAction()
{
    digitalWrite(ONBOARD_LED, LOW);
    digitalWrite(POWER_ON, LOW);
    FastLED.clear(true);
    FastLED.show(); // display this frame
    FastLED.delay(1000 / FRAMES_PER_SECOND);
    digitalWrite(ONBOARD_LED, HIGH);
    
}


// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  100

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 50
void Fire()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[ledHeight];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < ledHeight; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / ledHeight) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= ledHeight - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < ledHeight; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      for (int i=0; i<numColumns; i++)
      {
        posleds(pixelnumber,i)=color;
      }
    }
}