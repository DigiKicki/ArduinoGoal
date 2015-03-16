#define LETTER_G 6
#define LETTER_O 0
#define LETTER_A 0x77
#define LETTER_L 0x38
#define POS_LEFT_DIGIT_1 0
#define POS_LEFT_DIGIT_2 1
#define POS_RIGHT_DIGIT_1 3
#define POS_RIGHT_DIGIT_2 4
#define GOAL_ANIM_DELAY 500
#define GOAL_ANIM_DURATION 1500
#define GOAL_BLINK_DURATION 4000

void displayBlinkGOAL(Adafruit_7segment display) {
  display.writeDigitNum(POS_LEFT_DIGIT_1, LETTER_G);
  display.writeDigitNum(POS_LEFT_DIGIT_2, LETTER_O);
  display.writeDigitRaw(POS_RIGHT_DIGIT_1, LETTER_A);
  display.writeDigitRaw(POS_RIGHT_DIGIT_2, LETTER_L);
  display.drawColon(false);
  display.blinkRate(1);
  display.writeDisplay();
}

void displayAnimGOAL(Adafruit_7segment display) {
  display.writeDisplay();
  display.writeDigitNum(POS_RIGHT_DIGIT_2, LETTER_G);
  delay(GOAL_ANIM_DELAY);
  display.writeDigitNum(POS_RIGHT_DIGIT_1, LETTER_G);
  display.writeDigitNum(POS_RIGHT_DIGIT_2, LETTER_O);
  display.writeDisplay();
  delay(GOAL_ANIM_DELAY);
  display.drawColon(false);
  display.writeDigitNum(POS_LEFT_DIGIT_2, LETTER_G);
  display.writeDigitNum(POS_RIGHT_DIGIT_1, LETTER_O);
  display.writeDigitRaw(POS_RIGHT_DIGIT_2, LETTER_A);
  display.writeDisplay();
  delay(GOAL_ANIM_DELAY);
  display.writeDigitNum(POS_LEFT_DIGIT_1, LETTER_G);
  display.writeDigitNum(POS_LEFT_DIGIT_2, LETTER_O);
  display.writeDigitRaw(POS_RIGHT_DIGIT_1, LETTER_A);
  display.writeDigitRaw(POS_RIGHT_DIGIT_2, LETTER_L);
  display.writeDisplay();
  delay(GOAL_ANIM_DURATION);
}