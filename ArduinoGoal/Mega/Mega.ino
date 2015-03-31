
//////////////////////////////////////////////////////////////////////////////////
/// Dateiname: Mega.ino                                                        ///
/// Stand: 30.12.2014                                                          ///
/// Aufgaben: Beleuchtung, Spielstands-Anzeige, Torerkennung für 1 Tor         ///
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////---LIBRARIES---////////////////////////////////////////////////////////
#include <PinChangeInt.h>
#include <PinChangeIntConfig.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <analogComp.h>
#include <SerialCommunication.h>
#include <GoalAnimation7Segment.h>

/////////////////////////////////////////////////////---CONSTANTS---////////////////////////////////////////////////////////

  // CONSTANTS FOR LED STRIP 
#define LED_STRIP_PIN 4
#define LongSide 33
#define ShortSide 19
const int ANZ_LEDs = 2*(LongSide+ShortSide);
#define MidpointLongSideRight 17
#define MidpointLongSideLeft 69
#define MidpointShortSideBlue 43
#define MidpointShortSideYellow 95
#define internalDelayShowScoreDelay 100
#define random_Limit 3
  
  // CONSTANTS FOR 7SEG DISPLAY
#define GOAL_ANIM_DELAY 500
#define GOAL_ANIM_DURATION 1500

// CONSTANTS FOR ANALOG INTERRUPT
#define DIODE_PIN 0
#define GOAL_TIME_THRESHOLD 3000
#define PWM_PIN_REF_V 5
#define REFERENCE_VALUE 14

// CONSTANTS FOR BUTTONS
// Vorbereitung für Bedienpanel (6. Semester)
#define START_BUTTON_PIN 8
#define HOLD_TIME_THRESHOLD 3000
#define CONTACT_CHATTER_TIME 10

/////////////////////////////////////////////////////---VARIABLES---//////////////////////////////////////////////////////// 
  
  //  GERNERAL VARIABLES FOR CONTROL
  volatile boolean activeMatch = false;
  volatile boolean startMatch = false;
  volatile boolean restartMatch = false;
  volatile boolean stopMatch = false;
  
  //  VARIABLES FOR SERIAL COMMUNICATION
  byte incomingByte;
  byte GoalTimeMinutes;
  byte GoalTimeSeconds;
  
  //  VARIABLES FOR LED-STRIP  
  Adafruit_NeoPixel STRIP = Adafruit_NeoPixel(ANZ_LEDs, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
  int stripArray[ANZ_LEDs][3];
  int LastEvent = 0;
  volatile boolean SHOWSCORE = false;
  
    //Definition der Team-Farben Gelb und Blau im RGB-Code in 5 Stufen
    //Die Stufen werden für die Wellen verwendet, bei der die Helligkeit schrittweise herabgesetzt wird.
  int ColorTeamBlue[5][3]={{0, 0, 255}, {0, 0, 255/5*4}, {0, 0, 255/5*3}, {0, 0, 255/5*2}, {0, 0, 255/5*1}};
  int ColorTeamYellow[5][3]={{255, 180, 0}, {255/5*4, 180/5*4, 0}, {255/5*3, 180/5*3, 0}, {255/5*2, 180/5*2, 0}, {255/5*1, 180/5*1, 0}};
  
  //  VARIABLES FOR 7SEG DISPLAY
  Adafruit_7segment scoreDisplay = Adafruit_7segment();
  int ScoreTeamYellow = 0;
  int ScoreTeamBlue = 0;
  
  //  VARIABLES FOR ANALOG INTERRUPT
  volatile boolean goal_Yellow;
  volatile boolean goal_Blue;
  int DetectionCounter = 0;
  
  //  VARIABLES FOR BUTTONS
  long Start_HoldTime = 0;
  volatile boolean ScoreCorrected = false;
  
  
/////////////////////////////////////////////////////---SETUP---////////////////////////////////////////////////////////  
  
void setup()
{
  //  SETUP SERIAL COMMUNICATION
  Serial.begin(9600);    // startet serielle Konsole
  Serial1.begin(9600);   // startet serielle Kommunikation S1:  Pin19 (RX, Empfänger), Pin18 (TX, Sender)
  
  //  SETUP 7SEG DISPLAY
  scoreDisplay.begin(0x70);
  scoreDisplay.setBrightness(0);
  scoreDisplay.print(10000);        //Schreibt auf Display: - -:- -
  scoreDisplay.drawColon(true);     //Doppelpunkt wird eingeschaltet
  scoreDisplay.writeDisplay();
  
  //  SETUP LED STRIP
  STRIP.begin();
  STRIP.clear();    //komplettes Schieberegister wird geleert
  STRIP.show();     //Alle LEDs sind sicher ausgeschaltet
  
  // SETUP ANALOG INTERRUPT
  analogComparator.setOn(6, DIODE_PIN);
  analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
  analogWrite(PWM_PIN_REF_V, REFERENCE_VALUE);    //PWM-Signal für die Referenzspannung
  
  // SETUP TIMERS
  noInterrupts();           // disable all interrupts
  //Timer 1 for ShowScore()
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  OCR1A = 62500;         // compare match register 16MHz/256/2Hz
  TCCR1B |= _BV(WGM12);  // CTC mode
  TCCR1B |= _BV(CS12);   // 256 prescaler
  TIMSK1 |= _BV(OCIE1A); // enable timer compare interrupt
  interrupts();
  
  StartShow();
  /*for(int i=0; i<ANZ_LEDs; i++)
  {
    STRIP.setPixelColor(i, 255, 255, 255);
  }
  STRIP.show();*/
  startMatch = true;     //Spiel wird automatisch gestartet
}

/////////////////////////////////////////////////////---ISR---////////////////////////////////////////////////////////

ISR(TIMER1_COMPA_vect)
{
  //Tor-Erkennung freischalten
  if(goal_Blue == true)
  {
    DetectionCounter++;
    if(DetectionCounter == 3)
    {
      analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
      goal_Blue = false;
    }
  }
  
  //Zeit seit letztem Ereignis zählen, ggf. ShowScore freischalten
  if(activeMatch == true)  {LastEvent++; }
  if(LastEvent==120 && activeMatch==true && ScoreTeamYellow > 0 && ScoreTeamBlue > 0)      //alle 2 Minuten erscheint Spielstands-Effekt WENN mindestens 1 Tor erzielt wurde
  {
    SHOWSCORE = true;
  }
}

/////////////////////////////////////////////////////---LOOP---////////////////////////////////////////////////////////

void loop()
{
  //Tor-Abfrage über Boolean-Variabeln
  if(goal_Yellow)  { goal_Yellow=false; GOAL(true); }
  if(goal_Blue)    { GOAL(false); }
  
  //Abfrage ob Spielstand korregiert wurde
  if(ScoreCorrected)
  {
    WriteScoreOnDisplay(true);     //Spielstand Team Gelb aktualisieren
    WriteScoreOnDisplay(false);    //Spielstand Team Blau aktualisieren
    ScoreCorrected = false;
  }
  
  //Abfrage des Start-Buttons
  if(startMatch)  { startMatch = false; MatchStart(false); }
  if(restartMatch)  { restartMatch = false; MatchStart(true); }
  
  //Abfrage ob Spielende-Kommando über Serielle Kom. eingetroffen ist
  if(stopMatch)  { stopMatch = false; MatchEnd(); }
  
  //Abfrage ob 2 Minuten seit dem letzten Event vergangen sind und der Spielstand mit LEDs visualisiert werden soll
  if(SHOWSCORE)
  {
    SHOWSCORE = false;
    ShowScore();
  }
}

/////////////////////////////////////////////////////Serial-Kommunikation////////////////////////////////////////////////////////

void serialEvent1()
{
  incomingByte = Serial1.read();    //erstes Byte wird aus Buffer ausgelesen mit anschließender Fallunterscheidung
      
  switch(incomingByte)
  {
    case SERIAL_TIME_OVER: activeMatch=false; stopMatch=true; break;
    case SERIAL_GOAL_UNO: delay(5); GoalTimeMinutes=Serial1.read(); GoalTimeSeconds=Serial1.read(); goal_Yellow=true; break; //------<<Bool-Wert anpassen Teamabhängig
    case SERIAL_GOAL_ANSWER: delay(5); GoalTimeMinutes=Serial1.read(); GoalTimeSeconds=Serial1.read(); break;
    default: break;
  }
}


void SendSerialByte(byte sendByte)
{
  Serial1.print(sendByte);
  return;
}

/////////////////////////////////////////////////////---MORE_ISR---////////////////////////////////////////////////////////

void ISR_goalDetected() {
  analogComparator.disableInterrupt();
  goal_Blue = true; //--------------------<<variabel anpassen, teamabhängig
  SendSerialByte(SERIAL_GOAL_MEGA);
}

//-----------------------------------Vorbereitung für Bedienpanel (6. Semester)-----------------------------------------//

void ISR_scoreYellowIncrease()
{
  ScoreCorrected = true;
  ScoreTeamYellow++;
  //Schalterprellen blockieren:
  //Interrupt blockiert weiteren Int.->delay(max. Schalterprellzeit)
  delay(CONTACT_CHATTER_TIME);
}

void ISR_scoreYellowDecrease()
{
  ScoreCorrected = true;
  if(ScoreTeamYellow > 0)  { ScoreTeamYellow--; }
  //Schalterprellen blockieren:
  //Interrupt blockiert weiteren Int.->delay(max. Schalterprellzeit)
  delay(CONTACT_CHATTER_TIME);
}

void ISR_scoreBlueIncrease()
{
  ScoreCorrected = true;
  ScoreTeamBlue++;
  //Schalterprellen blockieren:
  //Interrupt blockiert weiteren Int.->delay(max. Schalterprellzeit)
  delay(CONTACT_CHATTER_TIME);
}

void ISR_scoreBlueDecrease()
{
  ScoreCorrected = true;
  if(ScoreTeamBlue > 0)  { ScoreTeamBlue--; }
  //Schalterprellen blockieren:
  //Interrupt blockiert weiteren Int.->delay(max. Schalterprellzeit)
  delay(CONTACT_CHATTER_TIME);
}


void ISR_Start_Button(void)
{
  noInterrupts(); 
  
  if(activeMatch)
  {
    SendSerialByte(SERIAL_TIME_RESET);
    restartMatch = true;
  }
  else if (!activeMatch)
  {
    SendSerialByte(SERIAL_TIME_START);
    startMatch = true;
  }
  
  delay(CONTACT_CHATTER_TIME);    //um Schalterprellen zu blockieren
  interrupts();
  return;
}

/////////////////////////////////////////////////////---7SEG DISPLAY---////////////////////////////////////////////////////////

void WriteScoreOnDisplay(boolean Team)
{
  int Score;
  byte pos1;
  byte pos2;
  
  if(Team)
  {
    Score = ScoreTeamYellow;
    pos1 = POS_LEFT_DIGIT_1;
    pos2 = POS_LEFT_DIGIT_2;
  }
  else
  {
    Score = ScoreTeamBlue;
    pos1 = POS_RIGHT_DIGIT_1;
    pos2 = POS_RIGHT_DIGIT_2;
  }
  
  byte digit1 = Score / 10;
  byte digit2 = Score % 10;
  
  scoreDisplay.writeDigitNum(pos1, digit1);
  scoreDisplay.writeDigitNum(pos2, digit2);
  scoreDisplay.drawColon(true);
  scoreDisplay.blinkRate(0);
  scoreDisplay.writeDisplay();
  return;
}

/////////////////////////////////////////////-----GOAL-----/////////////////////////////////////////////////

void GOAL(boolean Team)
{
  DecreaseBrightness(30);      //Helligkeit wird reduziert umd Tor-Show starten zu können
  
  WriteScoreOnDisplay(Team);   //Digiale Anzeige wird aktualisiert
    
  //Torshow-Effekt  
  switch(random(random_Limit))    //Aufruf der Tor-Show
  {
    case 0: LongSides2Waves(Team); break;
    case 1: BlinkingAndTwoWaves(Team); break;
    case 2: EveryOther(Team); break;
    case 3: ShotandBlinking(Team); break;
    default: break;
  }
  //Strip-Array vollständig auf Weis setzen
  setArrayColor(255, 255, 255);
  
  IncreaseBrightness(30);      //Helligkeit wird wieder angehoben um das Spiel fortzuführen
  LastEvent = 0;
}


/////////////////////////////////////////////ARRAY-FUNCTIONS/////////////////////////////////////////////////
void ClearLEDArray(void)
{
  for(int i=0; i<ANZ_LEDs; i++)  { stripArray[i][0] = 0; stripArray[i][1] = 0;stripArray[i][2] = 0;}
  return;
}

void ClearLEDArray(int Begin, int End, int ARRAY[][3])
{
  for(int i=Begin-1; i<=End-1; i++)  { ARRAY[i][0] = 0; ARRAY[i][1] = 0; ARRAY[i][2] = 0;}
  return;
}


void TransmiteToLEDs(void)
{
  for(int i=0; i<ANZ_LEDs; i++)
  {
    STRIP.setPixelColor(i, stripArray[i][0], stripArray[i][1], stripArray[i][2]);
  }
  STRIP.show();
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

void setArrayColorHalfHalf()
{
  setArrayColor(0, MidpointLongSideRight-1, stripArray, ColorTeamYellow[0][0], ColorTeamYellow[0][1], ColorTeamYellow[0][2]);
  setArrayColor(MidpointLongSideLeft, ANZ_LEDs-1, stripArray, ColorTeamYellow[0][0], ColorTeamYellow[0][1], ColorTeamYellow[0][2]);
  setArrayColor(MidpointLongSideRight, MidpointLongSideLeft-1-1, stripArray, ColorTeamBlue[0][0], ColorTeamBlue[0][1], ColorTeamBlue[0][2]);
  
  return;
}

///////////////////////////////////////////BRIGHTNESS-FUNCTIONS/////////////////////////////////////////////////////
void DecreaseBrightness (int delaytime)
{
  //Helligkeit Abdunkeln
  while(STRIP.getBrightness()>0)
  {
    //LEDs neu beschreiben
    for(int i=0; i<ANZ_LEDs; i++)  { STRIP.setPixelColor(i, stripArray[i][0], stripArray[i][1], stripArray[i][2]); }
    //Helligkeit Schrittweise Reduzieren
    if(STRIP.getBrightness()-10 < 0)  { STRIP.setBrightness(0); }
    else  { STRIP.setBrightness(STRIP.getBrightness()-10); }
    
    STRIP.show();
    delay(delaytime);
  }
  return;
}


void IncreaseBrightness (int delaytime)
{
  STRIP.setBrightness(0);
  
  //Helligkeit anheben
  while(STRIP.getBrightness()<255)
  {
    //LEDs neu beschreiben
    for(int i=0; i<ANZ_LEDs; i++) { STRIP.setPixelColor(i, stripArray[i][0], stripArray[i][1], stripArray[i][2]); }
    //Helligkeit Schrittweise Reduzieren
    if(STRIP.getBrightness()+10 > 255)  { STRIP.setBrightness(255); }
    else  { STRIP.setBrightness(STRIP.getBrightness()+10); }
    
    STRIP.show();
    delay(delaytime);
  }
  return;
}


////////////////////////////////////////-----Start- & End-Show-----////////////////////////////////////////////////////
void StartShow()
{
  //Eine Hälfte blau, andere Hälfte gelb
  for(int i=0; i<ANZ_LEDs; i++)
  {
    for(int j=0; j<3; j++)
    {
      if( i<=MidpointLongSideRight-1 || i>MidpointLongSideLeft-1)  { stripArray[i][j] = ColorTeamYellow[0][j]; }
      else  { stripArray[i][j] = ColorTeamBlue[0][j]; }
    }
  }
  
  IncreaseBrightness(60);
  Rotation360();
  Blinking(3,  250, 250);
  return;
}

//-------------------------------------------------------------------------------------------------------------------//

void MatchStart(boolean RESTART)
{
  activeMatch = true;
  
  if(RESTART)
  {
    DecreaseBrightness(60);
    scoreDisplay.setBrightness(0);
  }

  ScoreTeamYellow = 0;
  ScoreTeamBlue = 0;
  
  DecreaseBrightness(30);
  setArrayColor(255, 255, 255);    //Array auf Weis setzen
  scoreDisplay.setBrightness(15);  //Schaltet Display ein
  WriteScoreOnDisplay(true);
  WriteScoreOnDisplay(false);
  IncreaseBrightness(30);
}

void MatchEnd()
{
  activeMatch = false;
  EndShow();
  return;
}

void EndShow()
{
  //Abschluss-Show
  scoreDisplay.blinkRate(1);      //Display blinkt mit Endstand
  DecreaseBrightness(30);
  
  //LEDs in Sieger-Farbe einfärben, bei Unentschieden: Halb-Halb
  if(ScoreTeamYellow > ScoreTeamBlue)  { setArrayColor(ColorTeamYellow[0][0], ColorTeamYellow[0][1], ColorTeamYellow[0][2]); }
  else if(ScoreTeamYellow < ScoreTeamBlue)  { setArrayColor(ColorTeamBlue[0][0], ColorTeamBlue[0][1], ColorTeamBlue[0][2]); }
  else if(ScoreTeamYellow == ScoreTeamBlue)  { setArrayColorHalfHalf(); }
  
  IncreaseBrightness(30);
  Blinking(3,  250, 250);
  scoreDisplay.blinkRate(0);      //Ausschalten des Blinken des Displays
  delay(3000);
  
  scoreDisplay.print(10000);      //Schreibt auf Display: - -:- -
  scoreDisplay.setBrightness(0);  //Ausschalten des Displays
  DecreaseBrightness(60);
  
  setArrayColorHalfHalf();
  IncreaseBrightness(60);
  //Grundzustand bei nicht aktivem Spiel ist erreicht
  
  return;
}

//-------------------------------------------------------------------------------------------------------------------//

void Rotation360(void)
{
  //lässte aktuelles Bild einmal um 360° drehen
  
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
    
    TransmiteToLEDs();
    if(ANZ_Shift < ANZ_LEDs-1) { delay(60); }  //Nach letzem Durchgang wird nicht verzögert
  }
  return;
}


void Blinking(int ANZ, int ONtime, int OFFtime)
{
  for(int i=0; i<ANZ; i++)
  {
    delay(ONtime);     //Verzögerung LEDs eingeschaltet
    STRIP.clear();
    STRIP.show();      //LEDs-Ausschalten
    delay(OFFtime);    //Verzögerung LEDs ausgeschaltet
    TransmiteToLEDs(); //LEDs einschalten
  }
  return;
}

////////////////////////////////////////-----Score-Show-----////////////////////////////////////////////////////

void ShowScore(void)
{
  //Auswahl welcher ShowScore-Funktion verwendet werden kann
  if(ScoreTeamBlue <=10 && ScoreTeamYellow <= 10)  { ShowScoreLongSides(); }
  else if((ScoreTeamBlue >10 && ScoreTeamYellow >10) && (ScoreTeamBlue <19 && ScoreTeamYellow <19))  { ShowScoreShortSides(); }
  
  LastEvent = 0;    //Dauer seit dem letzten Event wird zurückgesetzt
}

void ShowScoreShortSides(void)
{
  //LEDs stufenweise in benötigtem Bereich von innen nach außen ausschalten
  for(int i=0; i<=9; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointShortSideBlue-1-i][j] = 0;
      stripArray[MidpointShortSideYellow-1+i][j] = 0;
      stripArray[MidpointShortSideBlue-1+i][j] = 0;
      stripArray[MidpointShortSideYellow-1-i][j] = 0;
    }
    TransmiteToLEDs();
    delay(internalDelayShowScoreDelay);
  }
  
  //Spielstand Team GELB auf Array schreiben
  for(int i=0; i<ScoreTeamYellow; i++)
  {
      for(int j=0; j<3; j++)  { stripArray[33+33+19-1+i][j] = ColorTeamYellow[0][j]; }
  }
  //Spielstand Team BLAU auf Array schreiben
  for(int i=0; i<ScoreTeamBlue; i++)
  {
    for(int j=0; j<3; j++)  { stripArray[33-1+i][j] = ColorTeamBlue[0][j]; }
  }
  TransmiteToLEDs();
  
  //alle 10 ms wird überprüft ob Signal zum Spielende angekommen ist
  //ja: wird Delay abgebrochen
  //nein: Delay für insgesamt 10 Sekunden
  int DelayCounter = 0;
  do
  {
    delay(10);
    DelayCounter++;
  } while( stopMatch==false && DelayCounter<(10000/10) );
  
  //Farbige LEDs im Bereich wieder ausschalten
  for(int i=9; i>=0; i++)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[33+33+19-1+i][j] = 0;
      stripArray[33-1+i][j] = 0;
    }
  }
  TransmiteToLEDs();
  
  //LEDs stufenweise in benötigtem Bereich von außen nach innen einschalten
  for(int i=9; i>=0; i--)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointShortSideBlue-1-i][j] = 255;
      stripArray[MidpointShortSideYellow-1-i][j] = 255;
      stripArray[MidpointShortSideBlue-1+i][j] = 255;
      stripArray[MidpointShortSideYellow-1+i][j] = 255;
    }
    TransmiteToLEDs();
    delay(internalDelayShowScoreDelay);
  }
  return;
}


void ShowScoreLongSides(void)
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
    TransmiteToLEDs();
    delay(internalDelayShowScoreDelay);
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
  TransmiteToLEDs();
  
  //alle 10 ms wird überprüft ob Signal zum Spielende angekommen ist
  //ja: wird Delay abgebrochen
  //nein: Delay für insgesamt 10 Sekunden
  int DelayCounter = 0;
  do
  {
    delay(10);
    DelayCounter++;
  } while( stopMatch==false && DelayCounter<(10000/10) );
  
  //Farbige LEDs im Bereich wieder ausschalten
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
  TransmiteToLEDs();
  
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
    TransmiteToLEDs();
    delay(internalDelayShowScoreDelay);
  }
  return;
}

//////////////////////////////////////////////////-----Tor-Shows----/////////////////////////////////////////////////////////
/// LongSides2Waves():                                                                       ///
/// Jeweils Zwei Wellen laufen an den langen Seiten zwei Mal synchron hin und zurück. Die    ///
/// Farbe entspricht der Torerzielenden Mannschaft.                                          ///
void LongSides2Waves(boolean Team)
{
  int TwinArray[LongSide][3];
  int WaveLength = 5;
  int WaveLEDon1[WaveLength];
  int WaveLEDon2[WaveLength];
  boolean WaveUpDown1[WaveLength];
  boolean WaveUpDown2[WaveLength];
  int NumbTurnsWave = 0;    //Zählt Anstöße am Seiten-Ende der vorderen Wellenfront der 1. Welle
  
  //Wellen-Arrays werden initialisiert
  Init_WaveArrays(WaveLength, WaveLEDon1, WaveUpDown1, LongSide, true);
  Init_WaveArrays(WaveLength, WaveLEDon2, WaveUpDown2, LongSide, false);

  ClearLEDArray();
  
  //Wellen-Bewegung
  while (NumbTurnsWave<4)
  {
    Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon1, WaveUpDown1, false, &NumbTurnsWave, true);
    Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon2, WaveUpDown2, false, &NumbTurnsWave, false);
    
    WriteStripArryTwoWaves(LongSide, TwinArray);
    TransmiteToLEDs();
    ClearLEDArray();
  
    delay(50);
  }  //Ende While-Schleife/Ende Torshow-Effekt
}

void WriteStripArryTwoWaves(int Limit_Array, int ARRAY[][3])
{
  //Limit_Array wird synchron auf beide lange Seiten kopiert
  
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
  //Direction: gibt die Bewegungsrichtung zu Beginn an
  for(int i=0; i<WaveLength; i++)
  {
    if(Direction == true)  { LEDon[i] = -i; }
    else  { LEDon[i] = Limit-1+i; }
    UpDown[i] = Direction;
  }
  return;
}

void Wave(boolean Team, int ArrayLength, int ARRAY[][3], int WaveLength, int LEDon[], boolean UpDown[], boolean PassLimit, int *NumbTurnsWave, boolean EnableCounting)
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
    
    //NumbTurnsWave +1 wenn Zählen freigegeben ist und Welle eine Strecke durchlaufen hat
    if (EnableCounting == true && i == 0 && (LEDon[0]==0 || LEDon[0]==ArrayLength-1)) { *NumbTurnsWave=*NumbTurnsWave+1; }
  }  //Ende For-Schleife/LEDs der Wellen neugesetzt
}

//-------------------------------------------------------------------------------------------------------------------//
/// BlinkingAndTwoWaves():                                                                   ///
/// Jeweils Zwei Wellen laufen an den langen Seiten zwei Mal hin und zurück. Zusätzlich      ///
/// blinkt die kurze Querseite hinter dem Tor, in das der Ball geschossen wird, in der Farbe ///
/// der Mannschaft, die das Tor erzielt.                                                     ///
void BlinkingAndTwoWaves(boolean Team)
{
  //Allgemeine Variabeln
  int InternalDelay = 10;
  int InternalCounterLongSides = 0;
  int InternalCounterShortSide = 0;
  boolean WriteStripArray = false;
  //Variabeln für lange Seiten
  int TwinArray[LongSide][3];    //Arbeits-Array: hier wird darauf gearbeitet und dann auf strip_Array kopiert
  int WaveLength = 5;
  int WaveLEDon1[WaveLength];    //merkt sich LED-Nummer auf dem Array für die 5 Wellen-LEDs
  int WaveLEDon2[WaveLength];
  boolean WaveUpDown1[WaveLength];    //gibt an in welche Richtung sich die einzelnen LEDs bewegen,
  boolean WaveUpDown2[WaveLength];    //wichtig bei Anstößen an Seiten-Ende
  int NumbTurnsWave = 0;    //Zählt Anstöße am Seiten-Ende der vorderen Wellenfront der 1. Welle
  //Variabeln für kurze Seite
  int ShortSideArray[ShortSide][3];
  boolean OnOff = true;
  
  //Wellen-Arrays werden initialisiert
  Init_WaveArrays(WaveLength, WaveLEDon1, WaveUpDown1, LongSide, true);
  Init_WaveArrays(WaveLength, WaveLEDon2, WaveUpDown2, LongSide, false);
  
  ClearLEDArray();
  
  //Wellen-Bewegung
  while (NumbTurnsWave < 4)
  {
    //Alle 50ms bewegt sich Welle weiter
    if(InternalCounterLongSides*InternalDelay >= 50)
    {
      Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon1, WaveUpDown1, false, &NumbTurnsWave, true);
      Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon2, WaveUpDown2, false, &NumbTurnsWave, false);
      
      WriteStripArryTwoWaves(LongSide, TwinArray);
      InternalCounterLongSides = 0;
      WriteStripArray = true;
    }
    
    //Alle 500ms schaltet kurze Seite um -> Blinken mit 2Hz
    if(InternalCounterShortSide*InternalDelay >= 500)
    {
      Blink(Team, ShortSide, ShortSideArray, &OnOff);
      InternalCounterShortSide = 0;
      WriteStripArray = true;
    }
    
    //Wenn etwas verändert wurde: LED-Band wird beschriebenund die langen Seiten gelöscht
    if(WriteStripArray == true)
    {
      TransmiteToLEDs();
      ClearLEDArray(1, LongSide, stripArray);
      ClearLEDArray(LongSide+ShortSide, 2*LongSide+ShortSide, stripArray);
      WriteStripArray = false;
    }
    
    InternalCounterLongSides++;
    InternalCounterShortSide++;
    delay(InternalDelay);
  }  //Ende While-Schleife/Ende Torshow-Effekt
  
  return;
}

//-------------------------------------------------------------------------------------------------------------------//
/// ShotandBlinking():                                                                       ///
/// an beiden langen Seiten jeweils eine Welle von dem gegenüberliegenden Ende auf die Seite ///
/// auf der das Tor erzielt wird zu. Erreicht die, Wellenfront das Ende fängt die Querseite  ///
/// an drei Mal zu blinken. Die Farbe entspricht der torerzielenden Mannschaft.              ///
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
  
  //Wellen-Array wird initialisiert
  Init_WaveArrays(WaveLength, WaveLEDon, WaveUpDown, LongSide, Team);
  
  ClearLEDArray();
  
  //Wellen-Bewegung
  while (NumbTurnsWave < 1 || NumbBlinking < 6)
  {
    //Alle 50ms bewegt sich Welle weiter
    if(InternalCounterLongSides*InternalDelay >= 50)
    {
      Wave(Team, LongSide, TwinArray, WaveLength, WaveLEDon, WaveUpDown, true, &NumbTurnsWave, true);
      
      WriteStripArryTwoWaves(LongSide, TwinArray);
      InternalCounterLongSides = 0;
      WriteStripArray = true;
    }
    
    //Alle 200ms schaltet kurze Seite, sobald Wellenfront Seiten-Ende erreicht hat
    if(NumbTurnsWave == 1 && InternalCounterShortSide*InternalDelay >= 200)
    {
      Blink(Team, ShortSide, ShortSideArray, &OnOff);
      NumbBlinking++;
      InternalCounterShortSide = 0;
      WriteStripArray = true;
    }
    
    //Wenn etwas verändert wurde: LED-Band wird beschriebenund die langen Seiten gelöscht
    if(WriteStripArray == true)
    {
      TransmiteToLEDs();
      ClearLEDArray(1, LongSide, stripArray);
      ClearLEDArray(LongSide+ShortSide, 2*LongSide+ShortSide, stripArray);
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

//-------------------------------------------------------------------------------------------------------------------//
/// EveryOther():                                                                            ///
/// klassischer Lichteffekt zum Beispiel auf Jahrmärkten, bei dem jeweilsdie benachbarten    ///
/// LEDs sich an bzw. ausschalten. Farbe entspricht der der Torerzielenden Mannschaft.       ///
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
    TransmiteToLEDs();
    delay(300);
  }
  return;
}
