#include <Adafruit_NeoPixel.h>

  #define PIN 6  
  #define ANZ_LEDs 40
  

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ANZ_LEDs, PIN, NEO_GRB + NEO_KHZ800);

uint32_t CurrentColorCode = strip.Color(0, 0, 0);
char* CurrentColor = {""};
uint32_t YellowCode = strip.Color(255, 200, 0);
uint32_t BlueCode = strip.Color(0, 0, 255);

void setup()
{
  Serial.begin(9600);
  
  strip.begin();
  strip.clear();
  strip.show();
}


void loop()
{
  for(int i=0; i<ANZ_LEDs; i++)
  {
    strip.setPixelColor(i, YellowCode);
  }
  strip.show();
}


void serialEvent()
{
  char read_byte = Serial.read();
  
  switch (read_byte)
  {
    case 'Y': CurrentColor = "Yellow"; CurrentColorCode = YellowCode; Serial.println("Yellow selected"); break;
    case 'B': CurrentColor = "Blue"; CurrentColorCode = BlueCode; Serial.println("Blue selected"); break;
    case 'S': SavePixelColor(); break;
    case 'p': PrintColor(); break;
    case '7': ChangeRedUp(); break;
    case '1': ChangeRedDown(); break;
    case '8': ChangeGreenUp(); break;
    case '2': ChangeGreenDown(); break;
    case '9': ChangeBlueUp(); break;
    case '3': ChangeBlueDown(); break;


  }
  return;
}

void SavePixelColor()
{
  if( CurrentColor == "Yellow")  { YellowCode = strip.getPixelColor(0); }
  else if( CurrentColor == "Blue")  { BlueCode = strip.getPixelColor(0); }
  Serial.println("Saved!");
  return;
}

void PrintColor()
{
  int Red = CurrentColorCode >> 16;
  int Green = (CurrentColorCode >> 8) && 255;
  int Blue = CurrentColorCode && 255;
  
  Serial.println(Red);
  Serial.println(Green);
  Serial.println(Blue);

  return;
}
  

void ChangeRedUp()
{
  int Red = CurrentColorCode >> 16;
  int Green = (CurrentColorCode >> 8) && 255;
  int Blue = CurrentColorCode && 255;
  
  Red = Red + 10;
  CurrentColorCode = strip.Color(Red, Green, Blue);
  Serial.println("Red increased");
  
  return;
}

void ChangeRedDown()
{
  int Red = CurrentColorCode >> 16;
  int Green = (CurrentColorCode >> 8) && 255;
  int Blue = CurrentColorCode && 255;
  
  Red = Red - 10;
  CurrentColorCode = strip.Color(Red, Green, Blue);
  Serial.println("Red decreased");
  
  return;
}

void ChangeGreenUp()
{
  int Red = CurrentColorCode >> 16;
  int Green = (CurrentColorCode >> 8) && 255;
  int Blue = CurrentColorCode && 255;
  
  Green = Green + 50;
  CurrentColorCode = strip.Color(Red, Green, Blue);
  Serial.println("Green increased");
  
  return;
}

void ChangeGreenDown()
{
  int Red = CurrentColorCode >> 16;
  int Green = (CurrentColorCode >> 8) && 255;
  int Blue = CurrentColorCode && 255;
  
  Green = Green - 10;
  CurrentColorCode = strip.Color(Red, Green, Blue);
  Serial.println("Green decreased");
  
  return;
}

void ChangeBlueUp()
{
  int Red = CurrentColorCode >> 16;
  int Green = (CurrentColorCode >> 8) && 255;
  int Blue = CurrentColorCode && 255;
  
  Blue = Blue + 10;
  CurrentColorCode = strip.Color(Red, Green, Blue);
  Serial.println("Blue increased");
  
  return;
}

void ChangeBlueDown()
{
  int Red = CurrentColorCode >> 16;
  int Green = (CurrentColorCode >> 8) && 255;
  int Blue = CurrentColorCode && 255;
  
  Blue = Blue - 10;
  CurrentColorCode = strip.Color(Red, Green, Blue);
  Serial.println("Blue decreased");
  
  return;
}
