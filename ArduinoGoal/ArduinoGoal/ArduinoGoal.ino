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
  //analogWrite(5, 1);
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
