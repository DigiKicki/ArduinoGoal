#include <Adafruit_NeoPixel.h>

  #define PIN 6  
  #define ANZ_LEDs 40
  
//void LEDsbeschreiben(void);
//void LEDarrayLeeren(void);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ANZ_LEDs, PIN, NEO_GRB + NEO_KHZ800);

  int stripArray[ANZ_LEDs];
  int LEDon[4]={3, 2, 1, 0};
  boolean UpDown[4]={true, true, true, true};
  
  int Brightness[]={255, 255/4*3, 255/2, 255/4};
  
  /*int BrightnessHigh = 255;
  int BrightnessMediumHigh = 255/4*3
  int BrightnessMediumLow = 255/2
  int BrightnessLow = 255/4*/

void setup()
{
  Serial.begin(9600);
  
  strip.begin();
  strip.show(); 
}


void loop()
{
  for(int i=0; i<4; i++)
  {
    if(LEDon[i]>=0)        //Sicherstellen dass im negativen Array-Bereich gearbeitet wird
    {
      if( Brightness[i] > stripArray[LEDon[i]] )        //Kontrolle ob neue Helligkeit größer ist als bisherige
      {
        stripArray[LEDon[i]] = Brightness[i];           //Helligkeitswert wird auf Strip-Array geschrieben
      }
      
      //LED-Nummer in./dekrementieren
      if (UpDown[i] == true) { LEDon[i]++; }
      else if (UpDown[i] == false) {LEDon[i]--; }
      
      //Richtung überprüfen und ggf. ändern
      if (LEDon[i] == 0) {UpDown[i] = true; }
      else if (LEDon[i] == (ANZ_LEDs-1)) {UpDown[i] = false; }
    }
  }
    
  LEDsbeschreiben();
  LEDarrayLeeren();
  
  delay(100);
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
    strip.setPixelColor(i, 0, 0, stripArray[i]);
  }
  strip.show();
  return;
}
  
