int LEDInaktiv=5; //Led wenn Lichtschranke nicht durchbrochen
int LEDAktiv=6; //Led wenn Lichtschranke durchbrochen
int PhotoDiode=0;
//Analoger Eingang der Photodiode
int value=0; //Messdaten
int grenzwert = 20;

void setup()
{
  Serial.begin(9600);
  
  // Set ACSR Register for analog interrupt
  ACSR = 
  (0<<ACD) |   // Analog Comparator: Enabled
  (0<<ACBG) |   // Analog Comparator Bandgap Select: AIN0 is applied to the positive input
  (0<<ACO) |   // Analog Comparator Output: Off
  (1<<ACI) |   // Analog Comparator Interrupt Flag: Clear Pending Interrupt
  (1<<ACIE) |   // Analog Comparator Interrupt: Enabled
  (0<<ACIC) |   // Analog Comparator Input Capture: Disabled
  (1<<ACIS1) | (1<ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge
  
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

}

ISR(ANALOG_COMP_vect)
{
  value=analogRead(PhotoDiode); //Photodiode auslesen
  Serial.println(value);
  if (value<grenzwert) //Wird der Lichtstrahl unterbrochen...
  {
    digitalWrite(LEDAktiv, HIGH);
    digitalWrite(LEDInaktiv, LOW);
    Serial.println("TOOOOOOOOOOOOOOOOOOOOOOR");
  }
  else
  {
    digitalWrite(LEDAktiv, LOW);
    digitalWrite(LEDInaktiv, HIGH);
  }
}
