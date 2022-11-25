#include <FastLED.h>
#include <SoftwareSerial.h>
bool off = true;

#define ONBOARD_LED 13
#define POWER_ON 7
#define POWER_SWITCH 2
#define NEXT 6
#define PREV 5

#define LED_PIN     A0
#define COLOR_ORDER GRB
#define CHIPSET     WS2813
#define NUM_LEDS    144

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60
bool gReverseDirection = false;



#define numColumns 6
#define ledHeight 24

CRGB leds[NUM_LEDS];
CRGB& posleds(int row, int col){
  int index =row+ledHeight*col;
  if (index > NUM_LEDS) {index=144;}
  if (index < 0) {index=0;};
  return leds[index];
}


void setup() {
Serial.begin(9600);//USB cable
 Serial.println("------setup start------");
  off = false;
  pinMode(POWER_ON, OUTPUT);
  pinMode(POWER_SWITCH, INPUT_PULLUP);
  pinMode(NEXT, INPUT_PULLUP);
  pinMode(PREV, INPUT_PULLUP);
  pinMode(ONBOARD_LED, OUTPUT);

  digitalWrite(POWER_ON, HIGH);
  digitalWrite(ONBOARD_LED, HIGH);
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  attachInterrupt(0,offcheck,CHANGE);
  delay(1000);
  Serial.println("------setup done------");
}

void loop() {
  if (!off){
    for ( int i = 0; i < ledHeight; i++) {  
      for (int j =0; j<numColumns; j++){
        if (j%2>0) {
        posleds(i,j) = CRGB::Blue;
        }
        else{posleds(i,j) = CRGB::Red;
        }
        
      }
      for (int k =0; k< ledHeight ; k++)
      {
        for (int j =0; j<numColumns; j++){
          posleds(k,j).fadeToBlackBy(abs(i-k)*100);
        }
      }
      FastLED.show(); // display this frame
      delay(1000);
    }
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  }

}


void offcheck()
{
  
  if (!digitalRead(10) or off) {
//    delay(5000);
    digitalWrite(ONBOARD_LED, LOW);
//    delay(5000);
    digitalWrite(POWER_ON, LOW);
    FastLED.clear(true);
    Serial.println("offcheck_off");
    off = true;
  }
  else
  {
    digitalWrite(ONBOARD_LED, HIGH);
    digitalWrite(POWER_ON, HIGH);
    // Serial.println("offcheck_on");
  }
}