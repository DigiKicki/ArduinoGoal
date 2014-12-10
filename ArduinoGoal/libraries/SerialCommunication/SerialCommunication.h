#define LETTER_G 6
#define LETTER_O 0
#define LETTER_A 0x77
#define LETTER_L 0x38
#define POS_LEFT_DIGIT_1 0
#define POS_LEFT_DIGIT_2 1
#define POS_RIGHT_DIGIT_1 3
#define POS_RIGHT_DIGIT_2 4
#define GOAL_BLINK_DURATION 4000

// BYTES FOR SERIAL COMMUNICATION


// Bytes send from UNO to MEGA
#define SERIAL_GOAL_UNO 0x3C		// next byte: minute, next byte second
#define SERIAL_TIME_OVER 0x3D
#define SERIAL_GOAL_ANSWER 0x41		// next byte: minute, next byte second

// Bytes send from MEGA to UNO
#define SERIAL_GOAL_MEGA 0x3E		// answer byte: minute, next byte second
#define SERIAL_TIME_START 0x3F
#define SERIAL_TIME_RESET 0x40
//#define SERIAL_TIME_CONFIG 0x42	// next byte: minute (if minute is 0 then use no time -> goal display)
