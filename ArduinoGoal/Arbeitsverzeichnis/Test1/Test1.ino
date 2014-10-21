#include <Adafruit_NeoPixel.h>

  #define PIN 6  
  #define ANZ_LEDS 1
  

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ANZ_LEDS, PIN, NEO_RGB + NEO_KHZ800);

void setup()
{
  Serial.begin(9600);
  
  strip.begin();
  strip.show();
  
  strip.setBrightness(255);
}


void loop()
{
  int Delay1 = 10;
  //Rot - Blau - Grün durchspringen
  Serial.println("Rot!");
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();
  delay(1000);
  
  Serial.println("Grün!");
  strip.setPixelColor(0, 0, 255, 0);
  strip.show();
  delay(1000);
  
  Serial.println("Blau!");
  strip.setPixelColor(0, 0, 0, 255);
  strip.show();
  delay(3000);
  
  strip.show();
  
  //Flüssiger Farbübergang
  
  for (int j=0; j<=12 ;j++)
  {
    if (j=0) {delay(3000); Serial.println("Farbspektrum");}
    if (j=7) {delay(3000); Serial.println("Aus-Rot-Aus-Grün-Aus-Blau-Aus");}
    
    for (int i=0; i<=255; i++)
    {
      if(j=0)  {strip.setPixelColor(0, 255, i, 0);}
      else if (j=1) {strip.setPixelColor(0, 255-i, 255, 0);}
      else if (j=2) {strip.setPixelColor(0, 0, 255, i);}
      else if (j=3) {strip.setPixelColor(0, 0, 255-i, 255);}
      else if (j=4) {strip.setPixelColor(0, i, 0, 255);}
      else if (j=5) {strip.setPixelColor(0, i, 0, 255-i);}
      else if (j=6) {strip.setPixelColor(0, 255-i, 0, 0);}
      
      else if (j=7) {strip.setPixelColor(0, i, 0, 0);}
      else if (j=8) {strip.setPixelColor(0, 255-i, 0, 0);}
      else if (j=9) {strip.setPixelColor(0, 0, i, 0);}
      else if (j=10) {strip.setPixelColor(0, 0, 255-i, 0);}
      else if (j=11) {strip.setPixelColor(0, 0, 0, i);}
      else if (j=12) {strip.setPixelColor(0, 0, 0, 255-i);}
      
      strip.show();
      delay(Delay1);
    }
  }
  
  strip.show();
  
  delay(3000);
  //Brightness-Test
  
  Serial.println("Helligkeitstest");
  for (int k=0; k<=255; k++)
  {
    strip.setPixelColor(0, 255, 0, 0);
    strip.setBrightness(255-k);
    
    strip.show();
    delay(Delay1);
  }
  
  for (int m=0; m<=255; m++)
  {
    strip.setPixelColor(0, 255, 0, 0);
    strip.setBrightness(m);
    
    strip.show();
    delay(Delay1);
  }
  delay(3000);
  
  //Weis-Test
  Serial.println("Weis-Test");
  strip.show();
  for (int n=0; n<=255; n++)
  {
    strip.setPixelColor(0, n, n, n);
        
    strip.show();
    delay(Delay1);
  }
  delay(3000);
  
}

