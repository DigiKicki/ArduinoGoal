// CONSTANTS BYTES FOR SERIAL COMMUNICATION


// Bytes send from UNO to MEGA
#define SERIAL_GOAL_UNO 0x3C		// next byte: minute, next byte second
#define SERIAL_TIME_OVER 0x3D
#define SERIAL_GOAL_ANSWER 0x41		// next byte: minute, next byte second

// Bytes send from MEGA to UNO
#define SERIAL_GOAL_MEGA 0x3E		// answer byte: minute, next byte second
#define SERIAL_TIME_START 0x3F
#define SERIAL_TIME_RESET 0x40
//#define SERIAL_TIME_CONFIG 0x42	// next byte: minute (if minute is 0 then use no time -> goal display)
