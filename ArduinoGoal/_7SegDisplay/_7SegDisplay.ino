#include <Adafruit_GFX.h>

#include <Wire.h>
#include <Adafruit_LEDBackpack.h>

Adafruit_7segment clockDisplay = Adafruit_7segment();
int i = 0;
volatile boolean second = false;

void setup() {
  Serial.begin(9600);
  //clockDisplay 
  clockDisplay.begin(0x71);
  
  TCCR1B=0;
  TCCR1A=0;
  TCNT1=0;
  
  OCR1A = 62500;
  //clockDisplay.writeDisplay();
  TCCR1B |= _BV(WGM12); // CTC mode
  TCCR1B |= _BV(CS12); // 256 prescaler
  TIMSK1 |= _BV(OCIE1A); // enable timer compare interrupt
  
  interrupts();
}

ISR(TIMER1_COMPA_vect)
{
  second = true;
  //i = (i==1) ? 0 : 1; 
  //Serial.println("qwe");
  //return;
  //digitalWrite(13, i);
  //Serial.println("asd");
  //return;
  //noInterrupts();
  
  //Serial.println("Sekunde");
  //interrupts();
}

void loop() {
  //delay(1000);
  if (second) {
    clockDisplay.print(i++);
    clockDisplay.drawColon(true);
    clockDisplay.writeDisplay();
    second = false;
  }
  //Serial.println("asd");
  //clockDisplay.print(i++);
  //clockDisplay.drawColon(true);
  //clockDisplay.writeDisplay();
}


