#include <Adafruit_NeoPixel.h>

  #define PIN 6  
  #define ANZ_LEDS 1
  

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ANZ_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  strip.begin();
  strip.show();
}


void loop()
{
  //Programm
}

