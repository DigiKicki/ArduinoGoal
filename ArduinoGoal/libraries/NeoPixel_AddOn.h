/*
  Test.h - Test library for Wiring - description
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// ensure this library description is only included once
#ifndef NEOPIXEL_ADDON_H
#define NEOPIXEL_ADDON_H

// include types & constants of Wiring core API
#include "WConstants.h"
#include "Adafruit_NeoPixel.h"

// library interface description
class NeoPixel_AddOn
{
  public:
	NeoPixel_AddOn( int, int, int);
	~NeoPixel_AddOn();

	Adafruit_NeoPixel strip = Adafruit_NeoPixel(2*(LongSide+ShortSide), PIN, NEO_GRB + NEO_KHZ800);

	int ANZ_LEDs = 2*(LongSide+ShortSide);

	int StripArray[ANZ_LEDs][3]


  public:
    Test(int);
    void LEDArrayLeeren(void)
	{
		for(int i=0; i<20; i++)  { StripArray[i][0] = 0; StripArray[i][1] = 0; StripArray[i][2] = 0;}
		return;
	};

  
};

#endif NEOPIXEL_ADDON_H

