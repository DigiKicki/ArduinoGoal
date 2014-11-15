#include <analogComp.h>
const int DIODE_PIN_1 = 0;
const int DIODE_PIN_2 = 1;
const int MAX_STEPS = 5;

int currentDiodeVal;
int beforeDiodeVal_1;
int beforeDiodeVal_2;
int stepCounter_1;
int stepCounter_2;

void setup()
{
  Serial.begin(9600);
  
  pinMode(DIODE_PIN_1, INPUT);
  pinMode(DIODE_PIN_2, INPUT);
  
  //beforeDiodeVal_1 = analogRead(DIODE_PIN_1);
  //beforeDiodeVal_2 = analogRead(DIODE_PIN_2);
}



void loop()
{
  checkDiodeValue(DIODE_PIN_1, &beforeDiodeVal_1, &stepCounter_1);
  checkDiodeValue(DIODE_PIN_2, &beforeDiodeVal_2, &stepCounter_2);
}

void serialEvent() {

}

void checkDiodeValue(int pin, int* beforeDiodeVal, int* counter) {
  currentDiodeVal = analogRead(pin);
  if (currentDiodeVal <= *beforeDiodeVal) {
    *counter++;
  } else {
    *counter = 0;
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
}
