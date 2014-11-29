#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>

// CONSTANTS FOR 7SEG DISPLAY
#define LETTER_G 6
#define LETTER_O 0
#define LETTER_A 0x77
#define LETTER_L 0x38
#define POS_LEFT_DIGIT_1 0
#define POS_LEFT_DIGIT_2 1
#define POS_RIGHT_DIGIT_1 3
#define POS_RIGHT_DIGIT_2 4
#define GOAL_BLINK_DURATION 3000
#define GOAL_ANIM_DELAY 500
#define GOAL_ANIM_DURATION 1500
// 7Seg variables
Adafruit_7segment clockDisplay = Adafruit_7segment();

// Goal variables
byte team1goals = 12;
byte team2goals = 43;

void setup() {
  Serial.begin(9600);
  
  // SETUP 7SEG DISPLAY
  clockDisplay.begin(0x71);
  setupDisplay();
  
  //TEST
  goalForTeam(true);
  goalForTeam(false);
  goalDisplayBlink();
  displayGoals();
  delay(1000);
  goalDisplayAnim();
  displayGoals();
}

void setupDisplay() {
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, 0);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, 0);
  clockDisplay.drawColon(true);
  clockDisplay.writeDisplay();
}

void loop() {
}

void goalForTeam(boolean team1) {
  byte goals;
  byte pos1;
  byte pos2;
  if (team1) {
    goals = ++team1goals;
    pos1 = POS_LEFT_DIGIT_1;
    pos2 = POS_LEFT_DIGIT_2;
  } else {
    goals = ++team2goals;
    pos1 = POS_RIGHT_DIGIT_1;
    pos2 = POS_RIGHT_DIGIT_2;
  }
  byte digit1 = goals / 10;
  byte digit2 = goals % 10;
  clockDisplay.writeDigitNum(pos1, digit1);
  clockDisplay.writeDigitNum(pos2, digit2);
  clockDisplay.drawColon(true);
  clockDisplay.blinkRate(0);
  clockDisplay.writeDisplay();
}

void goalDisplayBlink() {
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_1, LETTER_G);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, LETTER_O);
  clockDisplay.writeDigitRaw(POS_RIGHT_DIGIT_1, LETTER_A);
  clockDisplay.writeDigitRaw(POS_RIGHT_DIGIT_2, LETTER_L);
  clockDisplay.drawColon(false);
  clockDisplay.blinkRate(1);
  clockDisplay.writeDisplay();
  delay(GOAL_BLINK_DURATION);
}

void goalDisplayAnim() {
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, LETTER_G);
  clockDisplay.writeDisplay();
  delay(GOAL_ANIM_DELAY);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_1, LETTER_G);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, LETTER_O);
  clockDisplay.writeDisplay();
  delay(GOAL_ANIM_DELAY);
  clockDisplay.drawColon(false);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, LETTER_G);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_1, LETTER_O);
  clockDisplay.writeDigitRaw(POS_RIGHT_DIGIT_2, LETTER_A);
  clockDisplay.writeDisplay();
  delay(GOAL_ANIM_DELAY);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_1, LETTER_G);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, LETTER_O);
  clockDisplay.writeDigitRaw(POS_RIGHT_DIGIT_1, LETTER_A);
  clockDisplay.writeDigitRaw(POS_RIGHT_DIGIT_2, LETTER_L);
  clockDisplay.writeDisplay();
  delay(GOAL_ANIM_DURATION);
}

void displayGoals() {
  byte digit1 = team1goals / 10;
  byte digit2 = team1goals % 10;
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_1, digit1);
  clockDisplay.writeDigitNum(POS_LEFT_DIGIT_2, digit2);
  digit1 = team2goals / 10;
  digit2 = team2goals % 10;
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_1, digit1);
  clockDisplay.writeDigitNum(POS_RIGHT_DIGIT_2, digit2);
  clockDisplay.drawColon(true);
  clockDisplay.blinkRate(0);
  clockDisplay.writeDisplay();
}
