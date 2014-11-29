#include <WaveHC.h>
#include <WaveUtil.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <analogComp.h>

// CONSTANTS FOR ANALOG INTERRUPT
#define DIODE_PIN_1 0
#define DIODE_PIN_2 1
#define LED_PIN_1 9
#define LED_PIN_2 10
#define GOAL_TIME_THRESHOLD 3000
volatile boolean goal;
boolean mutex;

// VARIABLES FOR WAVE SHIELD
SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file 
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
#define playSound(name) playSound_P(PSTR(name))
#define SOUND_PLAY_TIME 2000

// CONSTANTS FOR 7SEG DISPLAY
#define POS_LEFT_DIGIT_1 0
#define POS_LEFT_DIGIT_2 1
#define POS_RIGHT_DIGIT_1 3
#define POS_RIGHT_DIGIT_2 4
#define SECOND 1000
// 7Seg variables
Adafruit_7segment clockDisplay = Adafruit_7segment();
long playTime;

//
long time;

void setup() {
  Serial.begin(9600);
  
  // SETUP 7SEG DISPLAY
  clockDisplay.begin(0x71);
  setupDisplay();
  
  // SETUP SD CARD
  setupWaveSDCard();

  // SETUP ANALOG INTERRUPT
  analogComparator.setOn(AIN0, DIODE_PIN_1);
  analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
  
  //TEST
  //analogWrite(11, 11);
  //goalDiaplayBlink();
  //goalTeam1();
  //playSound("asd.wav");
  //goal = true;
  //time = millis();
}

void loop(){
  //Serial.println("asd");
  
  if (goal && !mutex) {
    mutex = true;
    //asd();
    time = millis();
    
    playSound("FloMega.wav");
    
  }
  
  long currentTime = millis();
  
  if (currentTime - time > GOAL_TIME_THRESHOLD && mutex) {
    goal = mutex = false;
    analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
  }
  
  
  if (currentTime - playTime >= SECOND) {
    displayTime(currentTime);
    playTime = currentTime;
  }

  if (currentTime - time >= SOUND_PLAY_TIME) {
    wave.stop();
  }
}

void setupDisplay() {
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_1, 0);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, 0);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_1, 0);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, 0);
  clockDisplay.drawColon(true);
  clockDisplay.writeDisplay();
}

void displayTime(long time) {
  int TimerValueSec = time/1000;
  int DisplayTimeMin = TimerValueSec/60;
  int DisplayTimeSec = TimerValueSec-(DisplayTimeMin*60);
  byte digit1 = DisplayTimeMin / 10;
  byte digit2 = DisplayTimeMin % 10;
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_1, digit1);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, digit2);
  digit1 = DisplayTimeSec / 10;
  digit2 = DisplayTimeSec % 10;
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_1, digit1);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, digit2);
  clockDisplay.writeDisplay();
}

void setupWaveSDCard() {
  if (!card.init()){
    Serial.println("card.init failed");
  }
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
  if (!vol.init(card)){
    Serial.println("vol.init failed");
  }
  if (!root.openRoot(vol)){
    Serial.println("openRoot failed");
  }
}

void playSound_P(const char *name){
  char myname[13];
  // copy flash string to RAM
  strcpy_P(myname, name);
  // open file by name
  if (!file.open(root, myname)){
    Serial.println("open by name failed"); 
  }
  // create wave and start play
  if (!wave.create(file)){
    Serial.println("wave.create failed");
  }
  wave.play();
}

void ISR_goalDetected() {
  analogComparator.disableInterrupt();
  goal = true;
}
