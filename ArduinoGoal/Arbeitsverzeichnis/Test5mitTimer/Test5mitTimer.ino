#include <Adafruit_NeoPixel.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

 
  #define DELAY 50
  #define LongSide 33
  #define ShortSide 19
  #define MidpointLongSideRight 17
  #define MidpointLongSideLeft 69
  #define ANZ_LEDs 110

  Adafruit_NeoPixel strip = Adafruit_NeoPixel(2*(LongSide+ShortSide), 6, NEO_GRB + NEO_KHZ800);
  Adafruit_7segment clockDisplay = Adafruit_7segment();

  int stripArray[ANZ_LEDs][3];

  int TestCounter = 0;
  int GoalShow = 0;
  
  //Speicherung des Spielstands
  int ScoreTeamBlue = 3;
  int ScoreTeamYellow = 9;
  
  //Definition der Team-Farben Gelb und Blau im RGB-Code in 5 Stufen
  //Die Stufen werden für die Wellen verwendet, bei der die Helligkeit schrittweise herabgesetzt wird.
  int ColorTeamBlue[5][3]={{0, 0, 255}, {0, 0, 255/5*4}, {0, 0, 255/5*3}, {0, 0, 255/5*2}, {0, 0, 255/5*1}};
  int ColorTeamYellow[5][3]={{255, 180, 0}, {255/5*4, 180/5*4, 0}, {255/5*3, 180/5*3, 0}, {255/5*2, 180/5*2, 0}, {255/5*1, 180/5*1, 0}};
  
  //Timer-Variabeln
  int MatchTime= 0;
  int LastEvent = 0;
    int num=0;
  
void setup()
{
  Serial.begin(9600);  //Startet Serial-Monitor für die Programmier-Umgebung (STRG+Umschalt+M)
  pinMode(13, OUTPUT);
  pinMode(2, OUTPUT);
  attachInterrupt(0, WriteTimeOnDisplay, RISING);
  strip.begin();
  strip.clear();  //
  strip.show();
  
  clockDisplay.begin(0x71);
  
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  OCR1A = 62500;            // compare match register 16MHz/256/2Hz
  TCCR1B |= _BV(WGM12); // CTC mode
  TCCR1B |= _BV(CS12); // 256 prescaler
  TIMSK1 |= _BV(OCIE1A); // enable timer compare interrupt
  interrupts();
  
//  StartShow();
}

ISR(TIMER1_COMPA_vect)
{
  digitalWrite(13, digitalRead(13) ^ 1);
  
  MatchTime++;
  //digitalWrite(2, HIGH);
  //uWriteTimeOnDisplay(MatchTime);
}

void loop()
{

  for(int i=0; i<ANZ_LEDs; i++)
  {
    stripArray[i][0] = 255;
    stripArray[i][1] = 255;
    stripArray[i][2] = 255;
  }
  LEDsbeschreiben();
  num++;
  clockDisplay.print(num);
  clockDisplay.drawColon(true);
  clockDisplay.writeDisplay();
  
  delay(1000);
  
  //ShowScore();
  
  TestCounter++;
  if( TestCounter%2 == 0) { TOR(false); Serial.println("Team Blue"); }
  else { TOR(true); Serial.println("Team Yellow"); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////


void WriteTimeOnDisplay()
{
  int TimerValue=MatchTime;
  int TimerValueSec = TimerValue/1000;
  int DisplayTimeMin = TimerValueSec/60;
  int DisplayTimeSec = TimerValueSec-(DisplayTimeMin*60);
   
  clockDisplay.print(num);
  clockDisplay.drawColon(true);
  clockDisplay.writeDisplay();
  return;
}


void TOR(boolean Team)
{
  Serial.println("TOR");
  //LastEvent = Timer1.read()/1000;
  
  //Strip-Array vollständig auf Weis setzen
  setArrayColor(255, 255, 255);
  
  HelligkeitReduzieren(30);      //Helligkeit wird reduziert umd Tor-Show starten zu können
  
  //Zufallszahl generieren zu Auswahl des Torshow-Effekts
  //GoalShow = RandomGoalShow();
  
  //Torshow-Effekt  
  switch(GoalShow)    //Aufruf der Tor-Show
  {
    case 0: LongSides2Waves(Team); break;
    case 1: BlinkingAndTwoWaves(Team); break;
    case 2: EveryOther(Team); break;
    case 3: ShotandBlinking(Team); break;
  }
  //Strip-Array vollständig auf Weis setzen
  setArrayColor(255, 255, 255);
  
  HelligkeitAnheben(30);      //Helligkeit wird wieder angehoben um das Spiel fortzuführen
}

int RandomGoalShow(void)
{
  int Limit = 3;
  return(random(Limit));
}

/////////////////////////ARRAY-Funktionen/////////////////////////////
void LEDArrayLeeren(void)
{
  for(int i=0; i<ANZ_LEDs; i++)  { stripArray[i][0] = 0; stripArray[i][1] = 0;stripArray[i][2] = 0;}
  return;
}

void LEDArrayLeeren(int Begin, int End, int ARRAY[][3])
{
  for(int i=Begin-1; i<=End-1; i++)  { ARRAY[i][0] = 0; ARRAY[i][1] = 0; ARRAY[i][2] = 0;}
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

void setArrayColor(int Red, int Green, int Blue)
{
  for(int i=0; i<ANZ_LEDs; i++)
  {
    stripArray[i][0] = Red;
    stripArray[i][1] = Green;
    stripArray[i][2] = Blue;
  }
  return;
}

void setArrayColor(int Min_Limit, int Max_Limit, int ARRAY[][3], int Red, int Green, int Blue)
{
  for(int i=Min_Limit; i<Max_Limit; i++)
  {
    ARRAY[i][0] = Red;
    ARRAY[i][1] = Green;
    ARRAY[i][2] = Blue;
  }
  return;
}
/////////////////////////Helligkeits-Funktionen/////////////////////////////
void HelligkeitReduzieren (int delaytime)
{
  //Helligkeit Abdunkeln
  while(strip.getBrightness()>0)
  {
    //WEIS auf allen LEDs neu setzen
    for(int i=0; i<ANZ_LEDs; i++)  { strip.setPixelColor(i, stripArray[i][0], stripArray[i][1], stripArray[i][2]); }
    //Helligkeit Schrittweise Reduzieren
    if(strip.getBrightness()-10 < 0)  { strip.setBrightness(0); }
    else  { strip.setBrightness(strip.getBrightness()-10); }
    
    strip.show();
    delay(delaytime);
  }
  strip.setBrightness(255);  //Helligkeit wieder hochsetzen
  
  return;
}

void HelligkeitAnheben (int delaytime)
{
  strip.setBrightness(0);
  
  //Helligkeit anheben
  while(strip.getBrightness()<255)
  {
    //WEIS auf allen LEDs neu setzen
    for(int i=0; i<ANZ_LEDs; i++) { strip.setPixelColor(i, stripArray[i][0], stripArray[i][1], stripArray[i][2]); }
    //Helligkeit Schrittweise Reduzieren
    if(strip.getBrightness()+10 > 255)  { strip.setBrightness(255); }
    else  { strip.setBrightness(strip.getBrightness()+10); }
    
    strip.show();
    delay(delaytime);
  }
  return;
}


/////////////////////////Tor-Show-Funktionen/////////////////////////////

void LongSides2Waves(boolean Team)
{
  int TwinArray[LongSide][3];
  int WaveLength = 5;
  int WaveLEDon1[WaveLength];
  int WaveLEDon2[WaveLength];
  boolean WaveUpDown1[WaveLength];
  boolean WaveUpDown2[WaveLength];
  
  Init_WaveArrays(WaveLength, WaveLEDon1, WaveUpDown1, LongSide, true);
  Init_WaveArrays(WaveLength, WaveLEDon2, WaveUpDown2, LongSide, false);
    
  int Zaehler = 0;
  while (Zaehler<6)
  {
    Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon1, WaveUpDown1, false, &Zaehler, true);
    Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon2, WaveUpDown2, false, &Zaehler, false);
    
    WriteStripArryTwoWaves(LongSide, TwinArray);
    LEDsbeschreiben();
    LEDArrayLeeren();
  
    delay(DELAY);
  }  //Ende While-Schleife/Ende Torshow-Effekt
  
  return;
}

void WriteStripArryTwoWaves(int Limit_Array, int ARRAY[][3])
{
  for(int i=0; i<Limit_Array; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[i][j] = ARRAY[i][j];
      stripArray[LongSide+ShortSide+LongSide-1-i][j] = ARRAY[i][j];
      ARRAY[i][j] = 0;
    }
  }
  return;
}

void Init_WaveArrays(int WaveLength, int LEDon[], boolean UpDown[], int Limit, boolean Direction)
{
  for(int i=0; i<WaveLength; i++)
  {
    if(Direction == true)  { LEDon[i] = -i; }
    else  { LEDon[i] = Limit-1+i; }
    UpDown[i] = Direction;
  }
  return;
}

void Wave(boolean Team, int ArrayLength, int ARRAY[][3], int WaveLength, int LEDon[], boolean UpDown[], boolean PassLimit, int *Zaehler, boolean EnableCounting)
{
  
  //5 Einzel-LEDs einer Welle neu setzen
  for(int i=0; i<WaveLength; i++)
  {
    //Welle
    if( LEDon[i] >= 0 && LEDon[i]<=ArrayLength-1)        //Sicherstellen dass nicht im negativen Array-Bereich gearbeitet wird
    {
      if( Team == true )        //Farb-Auswahl: Gelb
      {
        if( ColorTeamYellow[i][0] > ARRAY[LEDon[i]][0] )        //Kontrolle ob neuer Farb größer ist als bisherige
        {
          for(int k=0; k<3; k++)      //Abarbeiten der drei RGB-Werte
          {
            ARRAY[LEDon[i]][k] = ColorTeamYellow[i][k];           //Farbwerte wird auf Strip-Array geschrieben
          }
        }
      }
      else        //Farb-Auswahl: Blau
      {
        if( ColorTeamBlue[i][2] > ARRAY[LEDon[i]][2] )        //Kontrolle ob neuer Farb größer ist als bisherige
        {
          for(int k=0; k<3; k++)      //Abarbeiten der drei RGB-Werte
          {
            ARRAY[LEDon[i]][k] = ColorTeamBlue[i][k];           //Farbwerte wird auf Strip-Array geschrieben
          }
        }
      }
    }
    //LED-Nummer in-/dekrementieren
    if (UpDown[i] == true) { LEDon[i]++; }
    else if (UpDown[i] == false) {LEDon[i]--; }
    
    //Richtung überprüfen und ggf. ändern
    if (PassLimit == false && LEDon[i] == 0) {UpDown[i] = true; }
    else if (PassLimit == false && LEDon[i] == (ArrayLength-1)) {UpDown[i] = false; }
    
    //Zaehler +1 wenn Zählen freigegeben ist und Welle eine Strecke durchlaufen hat
    if (EnableCounting == true && i == 0 && (LEDon[0]==0 || LEDon[0]==ArrayLength-1)) { *Zaehler=*Zaehler+1; Serial.println("Count!");}
  }  //Ende For-Schleife/LEDs der Wellen neugesetzt
  return;
}

////////////////////////////////////////-----Start-Show-----////////////////////////////////////////////////////
void StartShow()
{
  //Eine Hälfte blau, andere Hälfte gelb
  for(int i=0; i<ANZ_LEDs; i++)
  {
    for(int j=0; j<3; j++)
    {
      if( i<=MidpointLongSideRight || i>MidpointLongSideLeft)  { stripArray[i][j] = ColorTeamYellow[0][j]; }
      else  { stripArray[i][j] = ColorTeamBlue[0][j]; }
    }
  }
  
  HelligkeitAnheben(60);
  Rotation360();
  Blinking(3,  250, 250);
  HelligkeitReduzieren(60);
  //Array auf Weis setzen
  for(int i=0; i<ANZ_LEDs; i++)
  {
    for(int j=0; j<3; j++)  { stripArray[i][j] =255; }
  }
  HelligkeitAnheben(60);
  
  return;
}

void Rotation360(void)
{
  int buffer[3];
  
  for(int ANZ_Shift=0; ANZ_Shift<ANZ_LEDs; ANZ_Shift++)
  {
    buffer[0] = stripArray[0][0];
    buffer[1] = stripArray[0][1];
    buffer[2] = stripArray[0][2];
    
    for(int i=0; i<ANZ_LEDs-1; i++)
    {
      stripArray[i][0] = stripArray[i+1][0];
      stripArray[i][1] = stripArray[i+1][1];
      stripArray[i][2] = stripArray[i+1][2];
    }
    
    stripArray[ANZ_LEDs-1][0] = buffer[0];
    stripArray[ANZ_LEDs-1][1] = buffer[1];
    stripArray[ANZ_LEDs-1][2] = buffer[2];
    Serial.println(ANZ_Shift);
    LEDsbeschreiben();
    if(ANZ_Shift < ANZ_LEDs-1) { delay(60); }  //Nach letzem Durchgang wird nicht verzögert
  }
  return;
}

void Blinking(int ANZ, int ONtime, int OFFtime)
{
  for(int i=0; i<ANZ; i++)
  {
    delay(ONtime);     //Verzögerung LEDs eingeschaltet
    strip.clear();
    strip.show();      //LEDs-Ausschalten
    delay(OFFtime);    //Verzögerung LEDs ausgeschaltet
    LEDsbeschreiben(); //LEDs einschalten
  }
  return;
}

////////////////////////////////////////-----Score-Show-----////////////////////////////////////////////////////
void ShowScore(void)
{
  //LEDs stufenweise in benötigtem Bereich von innen nach außen ausschalten
  for(int i=0; i<=11; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointLongSideRight-1-i][j] = 0;
      stripArray[MidpointLongSideLeft-1+i][j] = 0;
      stripArray[MidpointLongSideRight-1+i][j] = 0;
      stripArray[MidpointLongSideLeft-1-i][j] = 0;
    }
    LEDsbeschreiben();
    delay(100);
  }
  //Spielstand Team GELB auf Array schreiben
  for(int i=0; i<ScoreTeamYellow; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointLongSideRight-1-1-i][j] = ColorTeamYellow[0][j];
      stripArray[MidpointLongSideLeft+1-1+i][j] = ColorTeamYellow[0][j];
    }
  }
  //Spielstand Team BLAU auf Array schreiben
  for(int i=0; i<ScoreTeamBlue; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointLongSideRight+1-1+i][j] = ColorTeamBlue[0][j];
      stripArray[MidpointLongSideLeft-1-1-i][j] = ColorTeamBlue[0][j];
    }
  }
  LEDsbeschreiben();
  delay(10000);
  
  //LEDs stufenweise im Bereich ausschalten
  for(int i=0; i<=11; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointLongSideRight-1-1-i][j] = 0;
      stripArray[MidpointLongSideLeft+1-1+i][j] = 0;
      stripArray[MidpointLongSideRight+1-1+i][j] = 0;
      stripArray[MidpointLongSideLeft-1-1-i][j] = 0;
    }
  }
  LEDsbeschreiben();
  
  //LEDs stufenweise in benötigtem Bereich von außen nach innen einschalten
  for(int i=11; i>=0; i--)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointLongSideRight-1-1-i][j] = 255;
      stripArray[MidpointLongSideLeft+1-1+i][j] = 255;
      stripArray[MidpointLongSideRight+1-1+i][j] = 255;
      stripArray[MidpointLongSideLeft-1-1-i][j] = 255;
    }
    LEDsbeschreiben();
    delay(100);
  }
  
  return;
}

////////////////////////////////////////-----Tor-Shows----////////////////////////////////////////////////////
void BlinkingAndTwoWaves(boolean Team)
{
  //Allgemeine Variabeln
  int InternalDelay = 10;
  int InternalCounterLongSides = 0;
  int InternalCounterShortSide = 0;
  boolean WriteStripArray = false;
  //Variabeln für lange Seiten
  int TwinArray[LongSide][3];
  int WaveLength = 5;
  int WaveLEDon1[WaveLength];
  int WaveLEDon2[WaveLength];
  boolean WaveUpDown1[WaveLength];
  boolean WaveUpDown2[WaveLength];
  int NumbTurnsWave = 0;
  //Variabeln für kurze Seite
  int ShortSideArray[ShortSide][3];
  boolean OnOff = true;
  
  
  Init_WaveArrays(WaveLength, WaveLEDon1, WaveUpDown1, LongSide, true);
  Init_WaveArrays(WaveLength, WaveLEDon2, WaveUpDown2, LongSide, false);
  
  LEDArrayLeeren();
  while (NumbTurnsWave < 4)
  {
    if(InternalCounterLongSides*InternalDelay >= DELAY)
    {
      Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon1, WaveUpDown1, false, &NumbTurnsWave, true);
      Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon2, WaveUpDown2, false, &NumbTurnsWave, false);
      
      WriteStripArryTwoWaves(LongSide, TwinArray);
      InternalCounterLongSides = 0;
      WriteStripArray = true;
    }
    
    if(InternalCounterShortSide*InternalDelay >= 500)
    {
      Blink(Team, ShortSide, ShortSideArray, &OnOff);
      InternalCounterShortSide = 0;
      WriteStripArray = true;
    }
    
    if(WriteStripArray == true)
    {
      LEDsbeschreiben();
      LEDArrayLeeren(1, LongSide, stripArray);
      LEDArrayLeeren(LongSide+ShortSide, 2*LongSide+ShortSide, stripArray);
      WriteStripArray = false;
    }
    
    InternalCounterLongSides++;
    InternalCounterShortSide++;
    delay(InternalDelay);
  }  //Ende While-Schleife/Ende Torshow-Effekt
  
  return;
}

void ShotandBlinking(boolean Team)
{
  //Allgemeine Variabeln
  int InternalDelay = 10;
  int InternalCounterLongSides = 0;
  int InternalCounterShortSide = 0;
  boolean WriteStripArray = false;
  //Variabeln für lange Seiten
  int TwinArray[LongSide][3];
  int WaveLength = 5;
  int WaveLEDon[WaveLength];
  boolean WaveUpDown[WaveLength];
  int NumbTurnsWave = 0;
  //Variabeln für kurze Seite
  int ShortSideArray[ShortSide][3];
  boolean OnOff = true;
  int NumbBlinking = 0;
  
  
  Init_WaveArrays(WaveLength, WaveLEDon, WaveUpDown, LongSide, Team);
  
  LEDArrayLeeren();
  while (NumbTurnsWave < 1 || NumbBlinking < 6)
  {
    if(InternalCounterLongSides*InternalDelay >= DELAY)
    {
      Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon, WaveUpDown, true, &NumbTurnsWave, true);
      
      WriteStripArryTwoWaves(LongSide, TwinArray);
      InternalCounterLongSides = 0;
      WriteStripArray = true;
    }
    
    if(NumbTurnsWave == 1 && InternalCounterShortSide*InternalDelay >= 200)
    {
      Blink(Team, ShortSide, ShortSideArray, &OnOff);
      NumbBlinking++;
      InternalCounterShortSide = 0;
      WriteStripArray = true;
    }
    
    if(WriteStripArray == true)
    {
      LEDsbeschreiben();
      LEDArrayLeeren(1, LongSide, stripArray);
      LEDArrayLeeren(LongSide+ShortSide, 2*LongSide+ShortSide, stripArray);
      WriteStripArray = false;
    }
    
    InternalCounterLongSides++;
    if(NumbTurnsWave == 1)  { InternalCounterShortSide++; }
    delay(InternalDelay);
  }  //Ende While-Schleife/Ende Torshow-Effekt
  
  return;
}

void Blink(boolean Team, int ArrayLength, int ARRAY[][3], boolean *OnOff)
{  
  for(int i=0; i<ArrayLength; i++)
  {
    for(int j=0; j<3; j++)
    {
      if(*OnOff == true && Team == true)  { ARRAY[i][j] = ColorTeamYellow[0][j]; }
      else if(*OnOff == true && Team == false)  { ARRAY[i][j] = ColorTeamBlue[0][j]; }
      else  { ARRAY[i][j] = 0; }
    }
  }
  if(Team == true)  { WriteBlueShortSideArray(ArrayLength, ARRAY); }
  else  { WriteYellowShortSideArray(ArrayLength, ARRAY); }
      
  if(*OnOff == true)   { *OnOff = false; }
  else  { *OnOff = true; }
  return;
}

void WriteBlueShortSideArray(int Limit_Array, int ARRAY[][3])
{
  for(int i=0; i<Limit_Array; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[i+LongSide][j] = ARRAY[i][j];
    }
  }
  return;
}

void WriteYellowShortSideArray(int Limit_Array, int ARRAY[][3])
{
  for(int i=0; i<Limit_Array; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[i+(2*LongSide)+ShortSide][j] = ARRAY[i][j];
    }
  }
  return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void EveryOther(boolean Team)
{
  boolean OnOff = false;
  int InternalCounter = 0;
  while(InternalCounter < 15)
  {
    if(InternalCounter%2 == 0)  { OnOff = true; }
    else  { OnOff = false; }
    
    for(int i=0; i<ANZ_LEDs; i++)
    {
      for(int j=0; j<3; j++)
      {
        if(OnOff == true && Team == true)   { stripArray[i][j] = ColorTeamYellow[0][j]; }
        else if(OnOff == true && Team == false)  { stripArray[i][j] = ColorTeamBlue[0][j]; }
        else  { stripArray[i][j] = 0; }
      }
      
      if(OnOff == true)   { OnOff = false; }
      else  {OnOff = true; }
    }
    InternalCounter++;
    LEDsbeschreiben();
    delay(300);
  }
  return;
}
