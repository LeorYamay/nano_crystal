#include <FastLED.h>

#define LED_PIN     A0
#define COLOR_ORDER GRB
#define CHIPSET     WS2813
#define NUM_LEDS    80

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60
int j = 0;
bool gReverseDirection = false;

CRGB leds[NUM_LEDS];
void setup() {
  pinMode(7, OUTPUT);
  pinMode(10, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  digitalWrite(7, HIGH);
  digitalWrite(13, HIGH);
  delay(1000);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  //  play_through_all();
  Serial.println("setup done");

}

void loop() {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
    FastLED.show(); // display this frame
    delay(100);
    j += 1;
    if (j > 20) {
      FastLED.clear();
      j = 0;
    }
    if (!digitalRead(10)) {
//    delay(5000);
    digitalWrite(13, LOW);
//    delay(5000);
    digitalWrite(7, LOW);
  }
  else
  {
    digitalWrite(13, HIGH);
    digitalWrite(7, HIGH);
  }
  }

  FastLED.delay(1000 / FRAMES_PER_SECOND);
//  if (!digitalRead(10)) {
////    delay(5000);
//    digitalWrite(13, LOW);
////    delay(5000);
//    digitalWrite(7, LOW);
//  }
//  else
//  {
//    digitalWrite(13, HIGH);
//    digitalWrite(7, HIGH);
//  }

}
