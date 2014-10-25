#include <NeoPixel_AddOn.h>

#include <Adafruit_NeoPixel.h>
//#include <NeoPixel_AddOn.h>


  #define PIN 6  
  #define DELAY 100
  #define LongSide 35
  #define ShortSide 20
  #define ANZ_LEDs 110
  
NeoPixel_AddOn STRIP = NeoPixel_AddOn(PIN, LongSide, ShortSide);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(2*(LongSide+ShortSide), PIN, NEO_GRB + NEO_KHZ800);


  int stripArray[ANZ_LEDs][3];
  
  int TestCounter = 0;
  
  int ColorTeamBlue[4][3]={{0, 0, 255}, {0, 0, 255/4*3}, {0, 0, 255/2}, {0, 0, 255/4}};
  int ColorTeamYellow[4][3]={{255, 255, 0}, {255/4*3, 255/4*3, 0}, {255/2, 255/2, 0}, {255/4, 255/4, 0}};
  
  


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
  
  TestCounter++;  if( TestCounter%2 == 0) { TOR(false); Serial.println("Team Blue"); }
  else { TOR(true); Serial.println("Team Yellow"); }
}


void TOR(boolean Team)
{
  Serial.println("TOR");
  
  WeisHelligkeitReduzieren();
  
  //Torshow-Effekt
  TwoWaves(Team);
  
  WeisHelligkeitAnheben();
}

/////////////////////////ARRAY-Funktionen/////////////////////////////
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

/////////////////////////Weis-Helligkeits-Funktionen/////////////////////////////
void WeisHelligkeitReduzieren (void)
{
  //Helligkeit-Weis Abdunkeln
  while(strip.getBrightness()>0)
  {
    //WEIS auf allen LEDs neu setzen
    for(int i=0; i<ANZ_LEDs; i++)  { strip.setPixelColor(i, 255, 255, 255); }
    //Helligkeit Schrittweise Reduzieren
    if(strip.getBrightness()-10 < 0)  { strip.setBrightness(0); }
    else  { strip.setBrightness(strip.getBrightness()-10); }
    
    strip.show();
    delay(30);
  }
  strip.setBrightness(255);  //Helligkeit wieder hochsetzen
  
  return;
}

void WeisHelligkeitAnheben (void)
{
  strip.setBrightness(0);
  
  //Helligkeit-Weis anheben
  while(strip.getBrightness()<255)
  {
    //WEIS auf allen LEDs neu setzen
    for(int i=0; i<ANZ_LEDs; i++)  { strip.setPixelColor(i, 255, 255, 255); }
    //Helligkeit Schrittweise Reduzieren
    if(strip.getBrightness()+10 > 255)  { strip.setBrightness(255); }
    else  { strip.setBrightness(strip.getBrightness()+10); }
    
    strip.show();
    delay(30);
  }
  return;
}


/////////////////////////Tor-Show-Funktionen/////////////////////////////
void TwoWaves(boolean Team)
{
  int LEDon1[4]={0, -1, -2, -3};
  int LEDon2[4]={ANZ_LEDs-1, ANZ_LEDs, ANZ_LEDs+1, ANZ_LEDs+2};
  boolean UpDown1[4]={true, true, true, true};
  boolean UpDown2[4]={false, false, false, false};
  
  int Zaehler = 0;
  while (Zaehler<3)
  {
    //Einzel-LEDs einer Welle neu setzen
    for(int i=0; i<4; i++)
    {
      //1. Welle
      if( LEDon1[i] >= 0 )        //Sicherstellen dass nicht im negativen Array-Bereich gearbeitet wird
      {
        if( Team == true )        //Farb-Auswahl: Gelb
        {
          if( ColorTeamYellow[i][0] > stripArray[LEDon1[i]][0] )        //Kontrolle ob neuer Farb größer ist als bisherige
          {
            stripArray[LEDon1[i]][0] = ColorTeamYellow[i][0];           //Farbwerte wird auf Strip-Array geschrieben
          }
        }
        else        //Farb-Auswahl: Blau
        {
          if( ColorTeamBlue[i][2] > stripArray[LEDon1[i]][2] )        //Kontrolle ob neuer Farb größer ist als bisherige
          {
            stripArray[LEDon1[i]][2] = ColorTeamBlue[i][2];           //Farbwerte wird auf Strip-Array geschrieben
          }
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
        if( Team == true )        //Farb-Auswahl: Gelb
        {
          if( ColorTeamYellow[i][0] > stripArray[LEDon2[i]][0] )        //Kontrolle ob neuer Farb größer ist als bisherige
          {
            stripArray[LEDon2[i]][0] = ColorTeamYellow[i][0];           //Farbwerte wird auf Strip-Array geschrieben
          }
        }
        else        //Farb-Auswahl: Blau
        {
          if( ColorTeamBlue[i][2] > stripArray[LEDon2[i]][2] )        //Kontrolle ob neuer Farb größer ist als bisherige
          {
            stripArray[LEDon2[i]][2] = ColorTeamBlue[i][2];           //Farbwerte wird auf Strip-Array geschrieben
          }
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
  }  //Ende While-Schleife/Ende Torshow-Effekt
  
  return;
}
