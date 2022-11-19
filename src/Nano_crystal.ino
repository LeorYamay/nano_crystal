#include <FastLED.h>

#define ONBOARD_LED 13
#define POWER_ON 7
#define POWER_SWITCH 10
#define NEXT 6
#define PREV 5

#define LED_PIN     A0
#define COLOR_ORDER GRB
#define CHIPSET     WS2813
#define NUM_LEDS    144

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60
int j = 0;
bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

void setup() {
  pinMode(POWER_ON, OUTPUT);
  pinMode(POWER_SWITCH, INPUT_PULLUP);
  pinMode(NEXT, INPUT_PULLUP);
  pinMode(PREV, INPUT_PULLUP);
  pinMode(ONBOARD_LED, OUTPUT);

  digitalWrite(POWER_ON, HIGH);
  digitalWrite(ONBOARD_LED, HIGH);
  delay(1000);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  Serial.println("setup done");
}

void loop() {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
    FastLED.show(); // display this frame
    delay(100);
    j += 1;
    for (int k =i; k>0 ; k--)
    {
      leds[k].fadeToBlackBy(abs(i-k));
    }
  
    offcheck();
  }

  FastLED.delay(1000 / FRAMES_PER_SECOND);

}


void offcheck()
{
  if (!digitalRead(10)) {
//    delay(5000);
    digitalWrite(ONBOARD_LED, LOW);
//    delay(5000);
    digitalWrite(POWER_ON, LOW);
  }
  else
  {
    digitalWrite(ONBOARD_LED, HIGH);
    digitalWrite(POWER_ON, HIGH);
  }
}