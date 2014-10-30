
#include <WaveHC.h>
#include <WaveUtil.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file 
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

#define playSound(name) playSound_P(PSTR(name))


void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps for debugging
  
  if (!card.init()){
    Serial.println("card.init failed");
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);

  if (!vol.init(card)){
    Serial.println("vol.init failed");
  }
  
  if (!root.openRoot(vol)){
    Serial.println("openRoot failed");
  }

}

void loop(){
  playSound("tor2.wav");
  delay(500);
}

void playSound_P(const char *name){
  char myname[13];
  
  // copy flash string to RAM
  strcpy_P(myname, name);

  // open file by name
  if (!file.open(root, myname)){
    Serial.println("open by name failed"); 
  }
  
  // create wave and start play
  if (!wave.create(file)){
    Serial.println("wave.create failed");
  }
  wave.play();
  while (wave.isplaying) {
    //playing
  }
  wave.stop();

}
