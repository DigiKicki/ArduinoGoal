int LEDInaktiv=5; //Led wenn Lichtschranke nicht durchbrochen
int LEDAktiv=6; //Led wenn Lichtschranke durchbrochen
int PhotoDiode=0;
//Analoger Eingang der Photodiode
int value=0; //Messdaten
int grenzwert = 20;
volatile boolean interrupt;

void setup()
{
  Serial.begin(9600);
  
  // Set ACSR Register for analog interrupt
  ACSR =  _BV (ACBG)    // set internal bandgap (1.1V)
        | _BV (ACI)     // (Clear) Analog Comparator Interrupt Flag
        | _BV (ACIE)    // Analog Comparator Interrupt Enable
        | _BV (ACIS1);  // ACIS1, ACIS0: Analog Comparator Interrupt Mode Select (trigger on falling edge)
  
  //benötigte Ports aktivieren
  pinMode(LEDAktiv, OUTPUT);
  pinMode(LEDInaktiv, OUTPUT);
  //pinMode(LEDIr, OUTPUT);
  pinMode(PhotoDiode, INPUT);
  //IR Led einschalten
  //digitalWrite(LEDIr,HIGH); //einschalten der IR Diode
  
}

void loop()
{
  if (interrupt) {
    interrupt = false;
    Serial.println("TOOOOOOOOOOOOOOOOOOOOOOR");
  }
}

ISR(ANALOG_COMP_vect)
{
  interrupt=true;
}
