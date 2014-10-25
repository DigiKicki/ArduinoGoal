#include <Adafruit_NeoPixel.h>

  #define PIN 6  
  #define ANZ_LEDs 10
  #define DELAY 100
  
//void LEDsbeschreiben(void);
//void LEDarrayLeeren(void);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ANZ_LEDs, PIN, NEO_GRB + NEO_KHZ800);

  int stripArray[ANZ_LEDs][3];
  int LEDon1[4]={0, -1, -2, -3};
  int LEDon2[4]={ANZ_LEDs-1, ANZ_LEDs, ANZ_LEDs+1, ANZ_LEDs+2};
  boolean UpDown1[4]={true, true, true, true};
  boolean UpDown2[4]={false, false, false, false};
  int Zaehler = 0;
  
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
  //Helligkeit-Weis Abdunkeln
  Serial.println("TOR");
  while(strip.getBrightness()>0)
  {
    //WEIS auf allen LEDs neu setzen
    for(int i=0; i<ANZ_LEDs; i++)  { strip.setPixelColor(i, 255, 255, 255); }
    //Helligkeit Schrittweise Reduzieren
    if(strip.getBrightness()-10<0)  { strip.setBrightness(0); }
    else  { strip.setBrightness(strip.getBrightness()-10); }
    
    strip.show();
    delay(30);
  }
  strip.setBrightness(255);  //Helligkeit wieder hochsetzen
  
  //Torshow-Effekt
  while (Zaehler<3)
  {
    //Einzel-LEDs einer Welle neu setzen
    for(int i=0; i<4; i++)
    {
      //1. Welle
      if(LEDon1[i]>=0)        //Sicherstellen dass im negativen Array-Bereich gearbeitet wird
      {
        if( Brightness[i] > stripArray[LEDon1[i]][2] )        //Kontrolle ob neue Helligkeit größer ist als bisherige
        {
          stripArray[LEDon1[i]][2] = Brightness[i];           //Helligkeitswert wird auf Strip-Array geschrieben
        }
      }
      //LED-Nummer in./dekrementieren
      if (UpDown1[i] == true) { LEDon1[i]++; }
      else if (UpDown1[i] == false) {LEDon1[i]--; }
      
      //Richtung überprüfen und ggf. ändern
      if (LEDon1[i] == 0) {UpDown1[i] = true; }
      else if (LEDon1[i] == (ANZ_LEDs-1)) {UpDown1[i] = false; }
      
      if ( i == 0 && LEDon1[0]==0 ) { Zaehler++; }
    
      //2. Welle
      if(LEDon2[i]<ANZ_LEDs)        //Sicherstellen dass im negativen Array-Bereich gearbeitet wird
      {
        if( Brightness[i] > stripArray[LEDon2[i]][2] )        //Kontrolle ob neue Helligkeit größer ist als bisherige
        {
          stripArray[LEDon2[i]][2] = Brightness[i];           //Helligkeitswert wird auf Strip-Array geschrieben
        }
      }
      //LED-Nummer in./dekrementieren
      if (UpDown2[i] == true) { LEDon2[i]++; }
      else if (UpDown2[i] == false) {LEDon2[i]--; }
      
      //Richtung überprüfen und ggf. ändern
      if (LEDon2[i] == 0) {UpDown2[i] = true; }
      else if (LEDon2[i] == (ANZ_LEDs-1)) {UpDown2[i] = false; }
    }  //Ende For-Schleife/LEDs der Wellen neugesetzt
    
  LEDsbeschreiben();
  LEDArrayLeeren();
  
  delay(DELAY);
  }  //Ende For-Schleife/Ende Torshow-Effekt
}

void LEDArrayLeeren(void)
{
  for(int i=0; i<ANZ_LEDs; i++)  { stripArray[i][0] = 0; stripArray[i][1] = 0;stripArray[i][2] = 0;}
  return;
}

void LEDsbeschreiben(void)
{
  for(int i=0; i<ANZ_LEDs; i++)
  {
    strip.setPixelColor(i, stripArray[i][0], stripArray[i][1], stripArray[i][2]);
  }
  strip.show();
  return;
}
  
