#include <analogComp.h>
const int DIODE_PIN_1 = 0;
const int DIODE_PIN_2 = 1;
const int DIODE_THRESHOLD = 5;
const int MAX_STEPS = 2;

int valueDiff;
int currentDiodeVal;
int beforeDiodeVal_1;
int beforeDiodeVal_2;
int stepCounter_1;
int stepCounter_2;

long time;

void setup()
{
  Serial.begin(9600);
  
  time = millis();
  
  pinMode(DIODE_PIN_1, INPUT);
  pinMode(DIODE_PIN_2, INPUT);
  
  TCCR1B |= _BV(WGM12); // CTC mode
  TCCR1B |= _BV(CS12); // 256 prescaler
  TIMSK1 |= _BV(OCIE1A); // enable timer compare interrupt

  
  analogReference(INTERNAL);
  beforeDiodeVal_1 = analogRead(DIODE_PIN_1);
  //beforeDiodeVal_2 = analogRead(DIODE_PIN_2);
}



void loop()
{
  int a = analogRead(0);
  Serial.println(a);
  delay(50);
  if ((millis() - time) > 1000) {
    //checkDiodeValue(DIODE_PIN_1, &beforeDiodeVal_1, &stepCounter_1);
    delay(10);
    //checkDiodeValue(DIODE_PIN_2, &beforeDiodeVal_2, &stepCounter_2);
    //delay(10);
  }
}

void serialEvent() {

}

void checkDiodeValue(int pin, int* beforeDiodeVal, int* counter) {
  currentDiodeVal = analogRead(pin);
  valueDiff = abs(*beforeDiodeVal - currentDiodeVal);
  if (valueDiff >= DIODE_THRESHOLD) {
    Serial.println(valueDiff);
    *counter += 1; 
  }
  *beforeDiodeVal = currentDiodeVal;
  
  if (*counter >= MAX_STEPS) {
    *counter = 0;
    goalDetected(pin);
  }
}

void goalDetected(int pin) {
  if (pin == DIODE_PIN_1) {
    //goal for team yellow
    Serial.println("TOOOOOOOOOOR_1");
  } else {
    //goal for team blue
    Serial.println("TOOOOOOOOOOR_2");
  }
  time = millis();
}

ISR(Timer1_COMPA_vect)
{
  Serial.println("Sekunde");
}

