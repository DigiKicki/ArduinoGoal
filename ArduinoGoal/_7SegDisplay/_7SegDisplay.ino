

//#include <Adafruit_GFX.h>

#include <Wire.h>
//#include <Adafruit_LEDBackpack.h>

#define SCL_PIN 2 
#define SCL_PORT PORTD 
#define SDA_PIN 0 
#define SDA_PORT PORTC 
//#include <SoftI2C.h>
#include <SoftI2CMaster.h>

//Adafruit_7segment clockDisplay;// = Adafruit_7segment();
int i = 0;
volatile boolean second = false;
int MatchTime, num;

void setup() {
  Serial.begin(9600);
  //clockDisplay 
  //clockDisplay.begin(0x71);
  
  /*TCCR1B=0;
  TCCR1A=0;
  TCNT1=0;
  
  OCR1A = 62500;
  //clockDisplay.writeDisplay();
  TCCR1B |= _BV(WGM12); // CTC mode
  TCCR1B |= _BV(CS12); // 256 prescaler
  TIMSK1 |= _BV(OCIE1A); // enable timer compare interrupt
  */
  //interrupts();
  
  if (!i2c_init()) {
    Serial.println("init");
  }
  if (!i2c_start(0x71 | I2C_WRITE)) {
    Serial.println("start");
  }
  byte val = 1;
  if (!i2c_write(val)) {
    Serial.println("write");
  }
  i2c_stop();
}

/*void WriteTimeOnDisplay()
{
  int TimerValue=MatchTime;
  int TimerValueSec = TimerValue/1000;
  int DisplayTimeMin = TimerValueSec/60;
  int DisplayTimeSec = TimerValueSec-(DisplayTimeMin*60);
   
  clockDisplay.print(num);
  clockDisplay.drawColon(true);
  clockDisplay.writeDisplay();
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
}*/

void loop() {
  delay(1000);
  
  
  
  //delay(1000);
  if (second && false) {
    //clockDisplay.print(i++);
    //clockDisplay.drawColon(true);
    //clockDisplay.writeDisplay();
    second = false;
  }
  //Serial.println("asd");
  //clockDisplay.print(i++);
  //clockDisplay.drawColon(true);
  //clockDisplay.writeDisplay();
}


