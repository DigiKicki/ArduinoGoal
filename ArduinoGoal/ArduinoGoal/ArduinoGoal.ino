#include <analogComp.h>

//int LEDInaktiv=5; //Led wenn Lichtschranke nicht durchbrochen
//int LEDAktiv=6; //Led wenn Lichtschranke durchbrochen
int PhotoDiode=0;
//Analoger Eingang der Photodiode
//int value=0; //Messdaten
//int grenzwert = 20;
volatile boolean goal;
long time = 0;

void setup()
{
  // AIN0 PIN6
  // AIN1 Pin7
  //380mV - 100mV
  // pwm -> 220mV
  //Timer 2 pwm pins: 3,11
  // prescaler auf min
  TCCR2B = TCCR2B & 0b11111000 | _BV(CS20);
  
  Serial.begin(9600);
  
  // enable analog comparator using internal reference voltage (1.1V) and analog pin 0 for input voltage
  analogComparator.setOn(AIN0, PhotoDiode);
  analogComparator.enableInterrupt(goalDetected, FALLING);
  
  //benötigte Ports aktivieren
  //pinMode(LEDAktiv, OUTPUT);
  //pinMode(LEDInaktiv, OUTPUT);
  //pinMode(LEDIr, OUTPUT);
  pinMode(PhotoDiode, INPUT);
  //IR Led einschalten
  //digitalWrite(LEDIr,HIGH); //einschalten der IR Diode
  analogWrite(3, 100);
  time = millis();
}

void loop()
{
  if (goal && (millis()-time) > 1000) {
    Serial.println("TOOOOOOOOOOOOOOOOOOOOOOR");
    time = millis();
    goal = false;
  }
}

void goalDetected() {
  goal = true;
}
