#include <Adafruit_NeoPixel.h>

  #define PIN 6  
  #define ANZ_LEDs 10
  
//void LEDsbeschreiben(void);
//void LEDarrayLeeren(void);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ANZ_LEDs, PIN, NEO_GRB + NEO_KHZ800);

  int stripArray[ANZ_LEDs];
  int LEDon = 0;
  boolean UpDown = true;
  
  /*int BrightnessHigh = 255;
  int BrightnessMedium = 255/3*2;
  int BrightnessLow = 255/3;*/

void setup()
{
  Serial.begin(9600);
  
  strip.begin();
  strip.show(); 
}


void loop()
{
  stripArray[LEDon] = 255;
  /*LEDon++;
  if (LEDon>ANZ_LEDs) {LEDon=0;}*/
  //Serial.println(UpDown);
  if (UpDown == true) { LEDon++; }
  else if (UpDown == false) {LEDon--; }
  
  if (LEDon == 0) {UpDown = true; }
  else if (LEDon == (ANZ_LEDs-1)) {UpDown = false; }
  
  LEDsbeschreiben();
  LEDarrayLeeren();
  
  delay(300);
}


void LEDarrayLeeren(void)
{
  for(int i=0; i<ANZ_LEDs; i++)
  {
    stripArray[i] = 0;
  }
  return;
}

void LEDsbeschreiben(void)
{
  for(int i=0; i<ANZ_LEDs; i++)
  {
    Serial.println(stripArray[i]);
    strip.setPixelColor(i, stripArray[i], 0, 0);
  }
  strip.show();
  return;
}
  
