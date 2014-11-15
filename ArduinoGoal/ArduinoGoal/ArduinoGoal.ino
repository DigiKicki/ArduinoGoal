#include <analogComp.h>
const int DIODE_PIN_1 = 0;
const int DIODE_PIN_2 = 1;
const int LED_PIN_1 = 9;
const int LED_PIN_2 = 10;
const int PWM_START_VAL = 255;

int PhotoDiode=0;

//int value=0; //Messdaten
//int grenzwert = 20;
volatile boolean goal;
boolean mutex = false;
long time = 0;
int pwm = 0;
int minvalue = 9999;

void setup()
{
  Serial.begin(9600);
  
  //analogWrite(LED_PIN_1, PWM_START_VAL);
  //analogWrite(LED_PIN_2, PWM_START_VAL);
  delay(50);
  //calibrateLedPwm();
  // AIN0 PIN6
  // AIN1 Pin7
  //380mV - 100mV
  // pwm -> 220mV
  //Timer 2 pwm pins: 3,11
  // prescaler auf min
  //TCCR2B = TCCR2B & 0b11111000 | _BV(CS20);
  
  
  
  pinMode(PhotoDiode, INPUT);

  
  //benötigte Ports aktivieren
  //pinMode(LEDAktiv, OUTPUT);
  
  //pinMode(LEDInaktiv, OUTPUT);
  //pinMode(LEDIr, OUTPUT);
  
  // enable analog comparator using reference voltage on PIN6 and analog pin 0 for input voltage
  analogComparator.setOn(AIN0, 1);
  analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
  //IR Led einschalten
  delay(50);
  int v = analogRead(0);
  Serial.println(v);
  //digitalWrite(LEDIr,HIGH); //einschalten der IR Diode
  //int val = (6.08*diode*1000/1023-25)/4.4;
  //Serial.println(val);
  //analogWrite(3, val);
  //1136mV
  //20mV
  //dV=1120
  // step = 4.4mV

  
  time = millis();
}



void loop()
{
  if (goal && !mutex) {
  int val = analogRead(0);
  if (val < minvalue ) {
    minvalue = val;
    Serial.println(minvalue);
  }
    Serial.println("TOOOOOOOOOOOOOOOOOOOOOOR");
    time = millis();
    mutex = true;
  }
  if ((millis()-time) > 3000) {
    goal = mutex = false;
  }
}

void serialEvent() {
  Serial.read();
  minvalue = 9999;
}

void ISR_goalDetected() {
  goal = true;
}
