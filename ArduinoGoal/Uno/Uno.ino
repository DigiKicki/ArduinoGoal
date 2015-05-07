#include <WaveHC.h>
#include <WaveUtil.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <analogComp.h>

#include <SerialCommunication.h>
#include <GoalAnimation7Segment.h>

#include <PinChangeInt.h>

// ANALOG INTERRUPT CONSTANTS
#define DIODE_PIN 0
#define PWM_PIN_REF_V 3
#define INTERRUPT_ENABLE_DURATION 1500
#define DIODE_VOLTAGE_OFFSET 5
// analog interrupt variables
volatile boolean goal1;
boolean reEnableInterrupr;

// WAVE SHIELD CONSTANTS
#define playSound(name) playSound_P(PSTR(name))
#define SOUND_PLAY_TIME 15000
// wave shield variables
SdReader card;
FatVolume vol;
FatReader root;
FatReader file; 
WaveHC wave;

// 7SEG CLOCK DISPLAY CONSTANTS
#define SECOND 1000.0
#define MINUTE 60.0
#define TEN 10.0
#define TIME_FIX_VALUE 900
// 7Seg clock display variables
Adafruit_7segment clockDisplay = Adafruit_7segment();
long playTime;
long startTime;
boolean goalBlink;

// serial communication variables
byte income;
boolean goal2;

// GAME CONSTANTS
#define START_PIN 8
#define RESET_TIMEOUT 3000
#define RESET_TIMEOUT_MAX 5000
// game variables
boolean startGame;
boolean gameStarted;
long currentTime;
long goalTime;
volatile long startTriggerTime;
volatile long startTriggerTimeEnd;

void setup() {
  Serial.begin(9600);
  
  // SETUP 7SEG DISPLAY
  clockDisplay.begin(0x71);
  setupDisplay();
  
  // SETUP SD CARD
  setupWaveSDCard();
  
  // get pwm value for reference voltage
  long diodeValue = analogRead(DIODE_PIN) - DIODE_VOLTAGE_OFFSET;
  analogWrite(PWM_PIN_REF_V, map(diodeValue, 0, 1023, 0, 255));
  
  // SETUP ANALOG INTERRUPT
  analogComparator.setOn(AIN0, DIODE_PIN);
  analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
  
  // SETUP START INTERRUPT
  pinMode(START_PIN, INPUT_PULLUP);
  PCintPort::attachInterrupt(START_PIN, ISR_startTrigger_pressed, FALLING);
  PCintPort::attachInterrupt(START_PIN, ISR_startTrigger_released, RISING);
}

void loop(){  
  // get current time only during a game
  if (gameStarted || wave.isplaying) {
    currentTime = millis();
  }

  if (startTriggerTime > 0) {
    if (!gameStarted) {
      Serial.write(SERIAL_GAME_START);
      startGame = true;
      startTriggerTime = 0;
    } else if (startTriggerTimeEnd - startTriggerTime > RESET_TIMEOUT) { // reset a running game
      Serial.write(SERIAL_GAME_RESET);
      startGame = true;
      startTriggerTimeEnd = startTriggerTime = 0;
    } else if (currentTime - startTriggerTime > RESET_TIMEOUT_MAX) {
      startTriggerTimeEnd = startTriggerTime = 0;
    }
  }
  
  // start a new game
  if (startGame) {
    setupDisplay();
    gameStarted = true;
    startGame = false;
    startTime = playTime = goalTime = currentTime = millis();
    playSound("start.wav");
  }
  
  // display game time when a game is started
  if (!goalBlink && gameStarted && currentTime - playTime >= SECOND) {
    displayTime(currentTime - startTime);
    playTime = currentTime;
  }
  
  // play goal sounds and send goal time
  if (gameStarted && (goal1 || goal2)) {
    goalTime = millis();
    if (goal1) {
      Serial.write(SERIAL_GOAL_UNO);
      playSound("tor1.wav");
      reEnableInterrupr = true;
    } else {
      Serial.write(SERIAL_GOAL_ANSWER);
      playSound("tor2.wav");
    }
    Serial.write(getMinute(goalTime - startTime));
    Serial.write(getSecond(goalTime - startTime));
    goal1 = goal2 = false;
    displayBlinkGOAL(clockDisplay);
    goalBlink = true;
  }
  
  // finish running game after 10 minutes, send signal and play sound
  if (gameStarted && currentTime - startTime > SECOND * MINUTE * TEN + TIME_FIX_VALUE) {
    Serial.write(SERIAL_TIME_OVER);
    gameStarted = false;
    goalTime = currentTime;
    playSound("ende.wav");
  }
  
  if (goalBlink && gameStarted && currentTime - goalTime >= GOAL_BLINK_DURATION) {
    clockDisplay.blinkRate(0);
    clockDisplay.writeDisplay();
    goalBlink = false;
  }
  
  // enable interrupt
  if (reEnableInterrupr && currentTime - goalTime >= INTERRUPT_ENABLE_DURATION) {
    analogComparator.enableInterrupt(ISR_goalDetected, FALLING);
    reEnableInterrupr = false;
  }
  
  // stop playing sound
  if (wave.isplaying && currentTime - goalTime >= SOUND_PLAY_TIME) {
    wave.stop();
  }
}

void serialEvent() {
  income = Serial.read();
  if (gameStarted && income == SERIAL_GOAL_MEGA) {
    goal2 = true;
  }
}

// init 7Seg clock with '00:00'
void setupDisplay() {
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_1, 0);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, 0);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_1, 0);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, 0);
  clockDisplay.drawColon(true);
  clockDisplay.writeDisplay();
}

// displays a time (time in millis)
void displayTime(long time) {
  byte digit1 = getMinute(time) / 10;
  byte digit2 = getMinute(time) % 10;
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_1, digit1);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, digit2);
  digit1 = getSecond(time) / 10;
  digit2 = getSecond(time) % 10;
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_1, digit1);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, digit2);
  clockDisplay.drawColon(true);
  clockDisplay.writeDisplay();
}

// setup wave shield and sd card
void setupWaveSDCard() {
  if (!card.init()){
    //Serial.println("failed to init SD Card");
  }
  card.partialBlockRead(true);
  if (!vol.init(card)){
    //Serial.println("failed to init volume");
  }
  if (!root.openRoot(vol)){
    //Serial.println("failed to open root folder");
  }
}

// play wave file by name
void playSound_P(const char *name){
  if (wave.isplaying) {
    wave.stop();
  }
  char myname[13];
  strcpy_P(myname, name);
  if (!file.open(root, myname)){
    //Serial.print("failed to open wave file: ");Serial.println(myname);
  }
  if (!wave.create(file)){
    //Serial.print("failed to create wave file: ");Serial.println(myname);
  }
  wave.play();
}

// interrupt service routine for made goals
void ISR_goalDetected() {
  analogComparator.disableInterrupt();
  goal1 = true;
}

byte getSecond(long time) {
  return byte((time/SECOND) - (getMinute(time)*MINUTE));
}

byte getMinute(long time) {
  return byte(time/(SECOND*MINUTE));
}

// interrupt service routine for start trigger
void ISR_startTrigger_released() {
  startTriggerTimeEnd = millis();
}

// interrupt service routine for start trigger
void ISR_startTrigger_pressed() {
  startTriggerTime = millis();
}
