#include <analogComp.h>

// CONSTANTS FOR ANALOG INTERRUPT
#define DIODE_PIN 0
#define GOAL_TIME_THRESHOLD 3000
#define PWM_PIN_REF_V 5
#define REFERENCE_VALUE 14


void setup()
{
  //  SETUP SERIAL COMMUNICATION
  Serial.begin(9600);    // startet serielle Konsole
  //Serial1.begin(9600);   // startet serielle Kommunikation S1:  Pin19 (RX, Empfänger), Pin18 (TX, Sender)
  
  
  
  // get pwm value for reference voltage
  long diodeValue = analogRead(DIODE_PIN);
  analogWrite(PWM_PIN_REF_V, map(diodeValue-5, 0, 1023, 0, 255));
  Serial.println(diodeValue);
  // SETUP ANALOG INTERRUPT
  analogComparator.setOn(AIN0, DIODE_PIN);
  analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
  
  
}

void loop()
{
  
}

void ISR_goalDetected() {
  //analogComparator.disableInterrupt();
  //goal_Blue = true; //--------------------<<variabel anpassen, teamabhängig
  //SendSerialByte(SERIAL_GOAL_MEGA);
  Serial.println("TOR");
}
