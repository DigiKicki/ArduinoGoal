int LEDInaktiv=5; //Led wenn Lichtschranke nicht durchbrochen
int LEDAktiv=6; //Led wenn Lichtschranke durchbrochen
int LEDIr=7; //IR Led
int PD=0; //Analoger Eingang der Photodiode
int val=0; //Messdaten
int do_debug=1;

void setup()
{
  if (do_debug ==1) Serial.begin(9600);
  //benötigte Ports aktivieren
  pinMode(LEDAktiv, OUTPUT);
  pinMode(LEDInaktiv, OUTPUT); 
  pinMode(LEDIr, OUTPUT);
  pinMode(PD, INPUT);
  //IR Led einschalten
  digitalWrite(LEDIr,HIGH); //einschalten der IR Diode
}

void loop()
{
  val=analogRead(PD); //Photodiode auslesen
  if (val<30) //Wird der Lichtstrahl unterbrochen...
  {
    digitalWrite(LEDAktiv, HIGH);
    digitalWrite(LEDInaktiv, LOW);
  } 
  else
  {
    digitalWrite(LEDAktiv, LOW);
    digitalWrite(LEDInaktiv, HIGH);
  }
  if (do_debug ==1) 
  {  
    Serial.println(val); //Werte zum Serial Monitor senden
    delay(500);  
  } 
} 
