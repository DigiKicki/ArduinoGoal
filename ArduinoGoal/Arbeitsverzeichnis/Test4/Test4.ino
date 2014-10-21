#include <Adafruit_NeoPixel.h>

  #define PIN 6  
  #define ANZ_LEDs 40
  
//void LEDsbeschreiben(void);
//void LEDarrayLeeren(void);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ANZ_LEDs, PIN, NEO_GRB + NEO_KHZ800);

  int stripArray[ANZ_LEDs];
  int LEDon1[4]={3, 2, 1, 0};
  int LEDon2[4]={ANZ_LEDs-4, ANZ_LEDs-3, ANZ_LEDs-2, ANZ_LEDs-1};
  boolean UpDown1[4]={true, true, true, true};
  boolean UpDown2[4]={false, false, false, false};
  
  int Brightness[]={255, 255/4*3, 255/2, 255/4};
  


void setup()
{
  Serial.begin(9600);
  
  strip.begin();
  strip.show(); 
}


void loop()
{
  for(int i=0; i<ANZ_LEDs; i++)
  {
    strip.setPixelColor(i, 255, 255, 255);
  }
  strip.show();
  
  delay(3000);
  
  TOR();
}


void TOR()
{
  Serial.println("TOR");
  while(strip.getBrightness()>0)
  {
    for(int i=0; i<ANZ_LEDs; i++)
    {
      strip.setPixelColor(i, 255, 255, 255);
    }
    if(strip.getBrightness()-10<0)
    {
      strip.setBrightness(0);
    }
    else
    {
      strip.setBrightness(strip.getBrightness()-10);
    }
    strip.show();
      
    delay(30);
  }
  
  
  strip.setBrightness(255);
  for(int j=0; j<100; j++)
  {
  for(int i=0; i<4; i++)
  {
    if(LEDon1[i]>=0)        //Sicherstellen dass im negativen Array-Bereich gearbeitet wird
    {
      if( Brightness[i] > stripArray[LEDon1[i]] )        //Kontrolle ob neue Helligkeit größer ist als bisherige
      {
        stripArray[LEDon1[i]] = Brightness[i];           //Helligkeitswert wird auf Strip-Array geschrieben
      }
      
      //LED-Nummer in./dekrementieren
      if (UpDown1[i] == true) { LEDon1[i]++; }
      else if (UpDown1[i] == false) {LEDon1[i]--; }
      
      //Richtung überprüfen und ggf. ändern
      if (LEDon1[i] == 0) {UpDown1[i] = true; }
      else if (LEDon1[i] == (ANZ_LEDs-1)) {UpDown1[i] = false; }
    }
    
    if(LEDon2[i]>=0)        //Sicherstellen dass im negativen Array-Bereich gearbeitet wird
    {
      if( Brightness[i] > stripArray[LEDon2[i]] )        //Kontrolle ob neue Helligkeit größer ist als bisherige
      {
        stripArray[LEDon2[i]] = Brightness[i];           //Helligkeitswert wird auf Strip-Array geschrieben
      }
      
      //LED-Nummer in./dekrementieren
      if (UpDown2[i] == true) { LEDon2[i]++; }
      else if (UpDown2[i] == false) {LEDon2[i]--; }
      
      //Richtung überprüfen und ggf. ändern
      if (LEDon2[i] == 0) {UpDown2[i] = true; }
      else if (LEDon2[i] == (ANZ_LEDs-1)) {UpDown2[i] = false; }
    }
  }
    
  LEDsbeschreiben();
  LEDarrayLeeren();
  
  delay(100);
  }
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
  strip.show();5
  return;
}
  