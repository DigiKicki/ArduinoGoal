/*
  Test.h - Test library for Wiring - description
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// ensure this library description is only included once
#ifndef NEOPIXEL_ADDON_H
#define NEOPIXEL_ADDON_H

// include types & constants of Wiring core API
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"

// library interface description
class NeoPixel_AddOn
{
  public:
	NeoPixel_AddOn( int PIN, int LongSide, int ShortSide);
	~NeoPixel_AddOn();

  private:
	boolean Team;

	int ANZ_LEDs;

	int StripArray[110][3];

	void LEDArrayLeeren(void);

	void LEDsbeschreiben(void);

public:

	int ColorTeamBlue[4][3];
	int ColorTeamYellow[4][3];

	void TOR(boolean Team);

/////////////////////////////Weis-Helligkeits-Funktionen/////////////////////////////
	void WeisHelligkeitReduzieren (void);

	void WeisHelligkeitAnheben (void);

/////////////////////////////Tor-Show-Funktionen/////////////////////////////
	void TwoWaves(boolean);

#endif //NEOPIXEL_ADDON_H

