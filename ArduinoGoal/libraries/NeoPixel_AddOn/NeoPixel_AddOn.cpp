#include "NeoPixel_AddOn.h"	

NeoPixel_AddOn::NeoPixel_AddOn( int PIN, int LongSide, int ShortSide)
{
	Adafruit_NeoPixel strip = Adafruit_NeoPixel(2*(LongSide+ShortSide), PIN, NEO_GRB + NEO_KHZ800);

	ANZ_LEDs = 2*(LongSide+ShortSide);


	//ColorTeamBlue={{0, 0, 255}, {0, 0, 255/4*3}, {0, 0, 255/2}, {0, 0, 255/4}};
	//ColorTeamYellow={{255, 255, 0}, {255/4*3, 255/4*3, 0}, {255/2, 255/2, 0}, {255/4, 255/4, 0}};
}

void LEDArrayLeeren(void)
{
	for(int i=0; i<20; i++)  { stripArray[i][0] = 0; stripArray[i][1] = 0; stripArray[i][2] = 0;}
	return;
}

void LEDsbeschreiben(void)
{
	for(int i=0; i<ANZ_LEDs; i++)
	{
		strip.setPixelColor(i, stripArray[i][0], stripArray[i][1], stripArray[i][2]);
	}
	strip.show();
	return;
}


void TOR(boolean Team)
{
	Serial.println("TOR");
 
	WeisHelligkeitReduzieren();
 
	//Torshow-Effekt
	TwoWaves(Team);
 
	WeisHelligkeitAnheben();
	return;
}

/////////////////////////////Weis-Helligkeits-Funktionen/////////////////////////////
	void WeisHelligkeitReduzieren (void)
	{
		//Helligkeit-Weis Abdunkeln
		while(strip.getBrightness()>0)
		{
			//WEIS auf allen LEDs neu setzen
			for(int i=0; i<ANZ_LEDs; i++)  { strip.setPixelColor(i, 255, 255, 255); }
			//Helligkeit Schrittweise Reduzieren
			if(strip.getBrightness()-10 < 0)  { strip.setBrightness(0); }
			else  { strip.setBrightness(strip.getBrightness()-10); }
    
			strip.show();
			delay(30);
		}
		strip.setBrightness(255);  //Helligkeit wieder hochsetzen
  
		return;
	}

	void WeisHelligkeitAnheben (void)
	{
		strip.setBrightness(0);
  
		//Helligkeit-Weis anheben
		while(strip.getBrightness() < 255)
		{
			//WEIS auf allen LEDs neu setzen
			for(int i=0; i<ANZ_LEDs; i++)  { strip.setPixelColor(i, 255, 255, 255); }
			//Helligkeit Schrittweise Reduzieren
			if(strip.getBrightness()+10 > 255)  { strip.setBrightness(255); }
			else  { strip.setBrightness(strip.getBrightness()+10); }
    
			strip.show();
			delay(30);
		}
		return;
	}


/////////////////////////////Tor-Show-Funktionen/////////////////////////////
	void TwoWaves(boolean Team)
	{
		int LEDon1[4]={0, -1, -2, -3};
		int LEDon2[4]={ANZ_LEDs-1, ANZ_LEDs, ANZ_LEDs+1, ANZ_LEDs+2};
		boolean UpDown1[4]={true, true, true, true};
		boolean UpDown2[4]={false, false, false, false};
  
		int Zaehler = 0;
		while (Zaehler<3)
		{
			//Einzel-LEDs einer Welle neu setzen
			for(int i=0; i<4; i++)
			{
				//1. Welle
				if( LEDon1[i] >= 0 )        //Sicherstellen dass nicht im negativen Array-Bereich gearbeitet wird
				{
					if( Team == true )        //Farb-Auswahl: Gelb
					{
						if( ColorTeamYellow[i][0] > stripArray[LEDon1[i]][0] )        //Kontrolle ob neuer Farb gr��er ist als bisherige
						{
							stripArray[LEDon1[i]][0] = ColorTeamYellow[i][0];           //Farbwerte wird auf Strip-Array geschrieben
						}
					}
					else        //Farb-Auswahl: Blau
					{
						if( ColorTeamBlue[i][2] > stripArray[LEDon1[i]][2] )        //Kontrolle ob neuer Farb gr��er ist als bisherige
						{
							stripArray[LEDon1[i]][2] = ColorTeamBlue[i][2];           //Farbwerte wird auf Strip-Array geschrieben
						}
					}
				}
				//LED-Nummer in./dekrementieren
				if (UpDown1[i] == true) { LEDon1[i]++; }
				else if (UpDown1[i] == false) {LEDon1[i]--; }
		
				//Richtung �berpr�fen und ggf. �ndern
				if (LEDon1[i] == 0) {UpDown1[i] = true; }
				else if (LEDon1[i] == (ANZ_LEDs-1)) {UpDown1[i] = false; }
		  
				if ( i == 0 && LEDon1[0]==0 ) { Zaehler++; }
		
				//2. Welle
				if(LEDon2[i]<ANZ_LEDs)        //Sicherstellen dass im negativen Array-Bereich gearbeitet wird
				{
					if( Team == true )        //Farb-Auswahl: Gelb
					{
						if( ColorTeamYellow[i][0] > stripArray[LEDon2[i]][0] )        //Kontrolle ob neuer Farb gr��er ist als bisherige
						{
							stripArray[LEDon2[i]][0] = ColorTeamYellow[i][0];           //Farbwerte wird auf Strip-Array geschrieben
						}
					}
					else        //Farb-Auswahl: Blau
					{
						if( ColorTeamBlue[i][2] > stripArray[LEDon2[i]][2] )        //Kontrolle ob neuer Farb gr��er ist als bisherige
						{
							stripArray[LEDon2[i]][2] = ColorTeamBlue[i][2];           //Farbwerte wird auf Strip-Array geschrieben
						}
					}
				}
				//LED-Nummer in./dekrementieren
				if (UpDown2[i] == true) { LEDon2[i]++; }
				else if (UpDown2[i] == false) {LEDon2[i]--; }
		
				//Richtung �berpr�fen und ggf. �ndern
				if (LEDon2[i] == 0) {UpDown2[i] = true; }
				else if (LEDon2[i] == (ANZ_LEDs-1)) {UpDown2[i] = false; }
			}  //Ende For-Schleife/LEDs der Wellen neugesetzt
    
			LEDsbeschreiben();
			LEDArrayLeeren();
  
			delay(DELAY);
		}  //Ende While-Schleife/Ende Torshow-Effekt
  
		return;
	}
};