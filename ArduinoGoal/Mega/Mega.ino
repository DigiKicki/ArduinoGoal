
//////////////////////////////////////////////////////////////////////////////////
/// Dateiname: Mega.ino                                                        ///
/// Stand: 15.05.2015                                                          ///
/// Aufgaben: Beleuchtung, Spielstands-Anzeige, Torerkennung für Tor GELB      ///
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////---LIBRARIES---////////////////////////////////////////////////////////
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

// CONSTANTS FOR BUTTONS
#define CONTACT_CHATTER_TIME 300

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
  volatile boolean AnalogCompareDisabled = false;
  int DiodeChatterCounter = 0;
  
  //  VARIABLES FOR BUTTONS
  volatile boolean ScoreCorrected = false;
  volatile unsigned long FirstContactTimeYellowIncrease = 0;
  volatile unsigned long FirstContactTimeYellowDecrease = 0;
  volatile unsigned long FirstContactTimeBlueIncrease = 0;
  volatile unsigned long FirstContactTimeBlueDecrease = 0;
  
  
/////////////////////////////////////////////////////---SETUP---////////////////////////////////////////////////////////  
  
void setup()
{
  //  SETUP SERIAL COMMUNICATION
  Serial.begin(9600);    // startet serielle Konsole
  Serial2.begin(9600);   // startet serielle Kommunikation S1:  Pin19 (RX, Empfänger), Pin18 (TX, Sender)
  
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
  analogComparator.setOn(INTERNAL_REFERENCE, DIODE_PIN);
  
  // LED FOR TIMER
  pinMode(13, OUTPUT);
  
  // SETUP BUTTONS //
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, ISR_scoreYellowIncrease, FALLING);    // Gelb+1 PIN: 2
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(1, ISR_scoreYellowDecrease, FALLING);    // Gelb-1 PIN: 3
  /*pinMode(19, INPUT_PULLUP);
  attachInterrupt(4, ISR_scoreBlueIncrease, FALLING);    // Blau+1 PIN: 19
  pinMode(18, INPUT_PULLUP);
  attachInterrupt(5, ISR_scoreBlueDecrease, FALLING);*/    // Blau-1 PIN: 18
  //auskommentiert aufgrund mechanischen Defekts des Schalters
  
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
  analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
  
  Serial.println("Setup abgeschlossen");  
}

/////////////////////////////////////////////////////---ISR---////////////////////////////////////////////////////////

ISR(TIMER1_COMPA_vect)
{
  digitalWrite(13, !digitalRead(13));
  //Tor-Erkennung freischalten
  if(AnalogCompareDisabled == true)
  {
    DiodeChatterCounter++;
    if(DiodeChatterCounter == 3)
    {
      analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
      Serial.println("Compare freigeschalten)");
      AnalogCompareDisabled = false;
      DiodeChatterCounter = 0;
    }
  }
  
  //Zeit seit letztem Ereignis zählen, ggf. ShowScore freischalten
  if(activeMatch == true)  {LastEvent++; }
  if(LastEvent==120 && activeMatch==true && (ScoreTeamYellow > 0 || ScoreTeamBlue > 0))      //alle 2 Minuten erscheint Spielstands-Effekt WENN mindestens 1 Tor erzielt wurde
  {
    SHOWSCORE = true;
  }
}

/////////////////////////////////////////////////////---LOOP---////////////////////////////////////////////////////////

void loop()
{
  //Abfrage ob Spielstand korregiert wurde
  if(ScoreCorrected)
  {
    WriteScoreOnDisplay(true);     //Spielstand Team Gelb aktualisieren
    WriteScoreOnDisplay(false);    //Spielstand Team Blau aktualisieren
    ScoreCorrected = false;
  }
  
  //Tor-Abfrage über Boolean-Variabeln
  if(goal_Yellow)  { goal_Yellow=false; ScoreTeamYellow++; WriteScoreOnDisplay(true); WriteScoreOnDisplay(false); GOAL(true); }
  if(goal_Blue)    { goal_Blue=false;  ScoreTeamBlue++; WriteScoreOnDisplay(true); WriteScoreOnDisplay(false); GOAL(false); }

  //Abfrage der Start-Merker
  if(startMatch)  { startMatch = false; Serial.println("MatchStart-Methode wird mit Start aufgerufen"); MatchStart(false); }
  if(restartMatch)  { restartMatch = false; Serial.println("MatchStart-Methode wird mit Restart aufgerufen"); MatchStart(true); }
  
  //Abfrage ob Spielende-Kommando über Serielle Kom. eingetroffen ist
  if(stopMatch)  { stopMatch = false; MatchEnd(); }
  
  //Abfrage ob 2 Minuten seit dem letzten Event vergangen sind und der Spielstand mit LEDs visualisiert werden soll
  if(SHOWSCORE)  { SHOWSCORE = false; ShowScore(); }
}

/////////////////////////////////////////////////////Serial-Kommunikation////////////////////////////////////////////////////////

void serialEvent2()
{
  incomingByte = Serial2.read();    //erstes Byte wird aus Buffer ausgelesen mit anschließender Fallunterscheidung
      
  switch(incomingByte)
  {
    case SERIAL_TIME_OVER: if(activeMatch) {activeMatch=false; stopMatch=true;} break;
    case SERIAL_GOAL_UNO: delay(5); Serial.println("Goalbyte angekommen"); GoalTimeMinutes=Serial2.read(); GoalTimeSeconds=Serial2.read(); goal_Yellow=true; break; //------<<Bool-Wert anpassen Teamabhängig
    case SERIAL_GOAL_ANSWER: delay(5); GoalTimeMinutes=Serial2.read(); GoalTimeSeconds=Serial2.read(); break;
    case SERIAL_GAME_START: if(!activeMatch && !startMatch) {startMatch=true; Serial.println("Start-Byte angekommen");} break;
    case SERIAL_GAME_RESET: {restartMatch=true; Serial.println("Reset-Byte angekommen");} break;
    default: break;
  }
}


void SendSerialByte(byte sendByte)
{
  Serial2.write(sendByte);
  Serial.println("Byte gesendet");
  return;
}

/////////////////////////////////////////////////////---MORE_ISR---////////////////////////////////////////////////////////

void ISR_goalDetected() {
  analogComparator.disableInterrupt();
  AnalogCompareDisabled=true;
  Serial.println("Tor-Routine");
  if(activeMatch)
  {
    goal_Blue = true; //--------------------<<variabel anpassen, teamabhängig
    Serial.println("Tor!");
    SendSerialByte(SERIAL_GOAL_MEGA);
  }
}

//--------------------------------------Bedienpanel Buttons--------------------------------------------//

void ISR_scoreYellowIncrease()
{
  //Schalterprellen blockieren
  if( activeMatch && (millis()-FirstContactTimeYellowIncrease) > CONTACT_CHATTER_TIME)
  {
    ScoreCorrected = true;
    ScoreTeamYellow++;
    Serial.println("Gelb+");;
    
    FirstContactTimeYellowIncrease = millis();
  }
  
}

void ISR_scoreYellowDecrease()
{
  //Schalterprellen blockieren
  if ( activeMatch && (millis()-FirstContactTimeYellowDecrease) > CONTACT_CHATTER_TIME)
  {
    ScoreCorrected = true;
    if(ScoreTeamYellow > 0)  { ScoreTeamYellow--; }
    Serial.println("Gelb-");
    
    FirstContactTimeYellowDecrease = millis();
  }
}

void ISR_scoreBlueIncrease()
{
  if ( activeMatch && (millis()-FirstContactTimeBlueIncrease) > CONTACT_CHATTER_TIME)
  {
    ScoreCorrected = true;
    ScoreTeamBlue++;
    Serial.println("Blue+");
    
    FirstContactTimeBlueIncrease = millis();
  }
}

void ISR_scoreBlueDecrease()
{
  if ( activeMatch && (millis()-FirstContactTimeBlueDecrease) > CONTACT_CHATTER_TIME)
  {
    ScoreCorrected = true;
    if(ScoreTeamBlue > 0)  { ScoreTeamBlue--; }
    Serial.println("Blue-");
    
    FirstContactTimeBlueDecrease = millis();
  }
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
  
  //DecreaseBrightness(30);      //Helligkeit wird reduziert umd Tor-Show starten zu können; Auskommentiert, da starkes Flimmern in der Beleuchtung
  
  WriteScoreOnDisplay(Team);   //Digiale Anzeige wird aktualisiert
  
  //Torshow-Effekt 
  if(Team)  { setArrayColor(255, 180, 0); Serial.println("Farbe auf Gelb");}
  else {setArrayColor(0, 0, 255); Serial.println("Farbe auf Blau");}
  Blinking(3,  250, 250);
  delay(1000);
   
  /*switch(random(random_Limit))    //Aufruf der Tor-Show
  {
    case 0: LongSides2Waves(Team); break;
    case 1: BlinkingAndTwoWaves(Team); break;
    case 2: EveryOther(Team); break;
    case 3: ShotandBlinking(Team); break;
    default: break;
  }*/
  //Auskommentiert, da starkes Flimmern in der Beleuchtung bis Totalausfall
  
  //Strip-Array vollständig auf Weis setzen
  setArrayColor(255, 255, 255);
  TransmiteToLEDs();
  
  //IncreaseBrightness(30);      //Helligkeit wird wieder angehoben um das Spiel fortzuführen; Auskommentiert, da starkes Flimmern in der Beleuchtung
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
  
  ///Rotation360(); //Auskommentiert, da starkes Flimmern in der Beleuchtung
  Blinking(3,  250, 250);
  return;
}

//-------------------------------------------------------------------------------------------------------------------//

void MatchStart(boolean RESTART)
{
  activeMatch = true;
  Serial.println("MatchStart");

  ScoreTeamYellow = 0;
  ScoreTeamBlue = 0;
  //Beleuchtung
  setArrayColor(255, 255, 255);    //Array auf Weis setzen
  TransmiteToLEDs();
  //Spielstandsanzeige
  scoreDisplay.setBrightness(15);  //Schaltet Display ein
  WriteScoreOnDisplay(true);
  WriteScoreOnDisplay(false);
}

void MatchEnd()
{
  activeMatch = false;
  EndShow();
  return;
}

void EndShow()
{
  Serial.println("Spielende");
  
  //Display: Abschluss-Show START
  scoreDisplay.blinkRate(1);      //Display blinkt mit Endstand
  //DecreaseBrightness(30);        //Auskommentiert, da starkes Flimmern in der Beleuchtung
  
  //LEDs in Sieger-Farbe einfärben, bei Unentschieden: Halb-Halb
  if(ScoreTeamYellow > ScoreTeamBlue)  { setArrayColor(ColorTeamYellow[0][0], ColorTeamYellow[0][1], ColorTeamYellow[0][2]); }
  else if(ScoreTeamYellow < ScoreTeamBlue)  { setArrayColor(ColorTeamBlue[0][0], ColorTeamBlue[0][1], ColorTeamBlue[0][2]); }
  else if(ScoreTeamYellow == ScoreTeamBlue)  { setArrayColorHalfHalf(); }
  
  //IncreaseBrightness(30);      //Auskommentiert, da starkes Flimmern in der Beleuchtung
  
  //Beleuchtung: Abschluss-Show
  Blinking(3,  500, 500);
  //Display: Abschluss-Show ENDE
  scoreDisplay.blinkRate(0);      //Ausschalten des Blinken des Displays
  delay(3000);
  
  //Display: Endzustand
  scoreDisplay.print(10000);      //Schreibt auf Display: - -:- -
  scoreDisplay.setBrightness(0);  //Ausschalten des Displays
  
  //Beleuchtung: Endzustand
  //DecreaseBrightness(60);    //Auskommentiert, da starkes Flimmern in der Beleuchtung
  setArrayColorHalfHalf();
  TransmiteToLEDs();
  //IncreaseBrightness(60);    //Auskommentiert, da starkes Flimmern in der Beleuchtung
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
  //else if((ScoreTeamBlue >10 || ScoreTeamYellow >10) && (ScoreTeamBlue <19 && ScoreTeamYellow <19))  { ShowScoreShortSides(); }
  //Auskommentiert, da nicht funktionsfähig
  
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
      stripArray[MidpointLongSideRight-1-i][j] = 0;
      stripArray[MidpointLongSideLeft-1+i][j] = 0;
      stripArray[MidpointLongSideRight-1+i][j] = 0;
      stripArray[MidpointLongSideLeft-1-i][j] = 0;
    }
  }
  TransmiteToLEDs();
  
  //LEDs stufenweise in benötigtem Bereich von außen nach innen einschalten
  for(int i=11; i>=0; i--)
  {
    for(int j=0; j<3; j++)
    {
      stripArray[MidpointLongSideRight-1-i][j] = 255;
      stripArray[MidpointLongSideLeft-1+i][j] = 255;
      stripArray[MidpointLongSideRight-1+i][j] = 255;
      stripArray[MidpointLongSideLeft-1-i][j] = 255;
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
