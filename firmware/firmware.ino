#include <Adafruit_Trellis.h>

/*************************************************** 
  This is a test example for the Adafruit Trellis w/HT16K33

  Designed specifically to work with the Adafruit Trellis 
  ----> https://www.adafruit.com/products/1616
  ----> https://www.adafruit.com/products/1611

  These displays use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include "Adafruit_Trellis.h"

#define NB_CHANNELS   12
#define NB_SOUNDSET   8
#define NB_MOOD       12
#define NB_MOODSOUND  12
#define NB_THEMESOUND 12
#define NB_GENERAL    4 //Do not change [MUTE] [-] [+] [Play/Pause] 

//Define 6 Matrix 4x4
Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_Trellis matrix1 = Adafruit_Trellis();
Adafruit_Trellis matrix2 = Adafruit_Trellis();
Adafruit_Trellis matrix3 = Adafruit_Trellis();
Adafruit_Trellis matrix4 = Adafruit_Trellis();
Adafruit_Trellis matrix5 = Adafruit_Trellis();

//Define trelis
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0, &matrix1, &matrix2, &matrix3, &matrix4, &matrix5);

// set to however many you're working with here, up to 8
#define NUMTRELLIS 6
#define numKeys (NB_CHANNELS * 4 + NB_SOUNDSET + NB_MOOD + NB_MOODSOUND + NB_THEMESOUND + NB_GENERAL)

#if numKeys > NUMTRELLIS*16
  #error "Too many input define, check NUMTRELLIS and NB_* definition"
#endif

enum theSpeed { UNDEF, SLOW, MEDIUM, FAST };

int position;

int channelMute[NB_CHANNELS]={};
int channelMin[NB_CHANNELS]={};
int channelMax[NB_CHANNELS]={};
int channelSingle[NB_CHANNELS]={};

int soundSet[NB_SOUNDSET]={};
int mood[NB_MOOD]={};
int moodSound[NB_MOODSOUND]={};
int themeSound[NB_THEMESOUND]={};
int general[NB_GENERAL]={};

int saved[NB_CHANNELS]={};

// Connect Trellis Vin to 5V and Ground to ground.
// Connect the INT wire to pin #A2 (can change later!)
#define INTPIN A2
// Connect I2C SDA pin to your Arduino SDA line
// Connect I2C SCL pin to your Arduino SCL line
// All Trellises share the SDA, SCL and INT pin! 
// Even 8 tiles use only 3 wires max

int in_array(int needle, int theArray[], int arraySize){
  for (int x = 0; x < arraySize; x++){
    if (theArray[x] == needle){
      return x+1;
    }    
  }
  return 0;
}

void initKey(){
  for (uint8_t i=0; i<NB_CHANNELS; i++) {
    channelMute[i]   = 4*i+0;
    channelMin[i]    = 4*i+1;
    channelMax[i]    = 4*i+2;
    channelSingle[i] = 4*i+3;
  }

  for (uint8_t i=0; i<NB_SOUNDSET; i++) {
    soundSet[i] = 4*NB_CHANNELS+i;
  }
  for (uint8_t i=0; i<NB_MOOD; i++) {
    mood[i] = 4*NB_CHANNELS+NB_SOUNDSET+i;
  }
  for (uint8_t i=0; i<NB_MOODSOUND; i++) {
    moodSound[i] = 4*NB_CHANNELS+NB_SOUNDSET+NB_MOOD+i;
  }
  for (uint8_t i=0; i<NB_THEMESOUND; i++) {
    themeSound[i] = 4*NB_CHANNELS+NB_SOUNDSET+NB_MOOD+NB_MOODSOUND+i;
  }
  for (uint8_t i=0; i<NB_GENERAL; i++) {
    general[i] = 4*NB_CHANNELS+NB_SOUNDSET+NB_MOOD+NB_MOODSOUND+NB_THEMESOUND+i;
  }
}

void blinkLed( int keyId, theSpeed speed = MEDIUM ){
  int theDelay;
  int nbX;
  switch (speed) {
    case SLOW:
      theDelay = 100;
      nbX = 2;
      break;
    case FAST:
      theDelay = 10;
      nbX = 10;
      break;
    case MEDIUM:
    default:
      theDelay = 25;
      nbX = 6;
    break;
  }

  for (uint8_t i=0; i<nbX; i++) {
    if (trellis.isLED(keyId)){
      trellis.clrLED(keyId);
    }else{
      trellis.setLED(keyId);
    }
    trellis.writeDisplay();
    delay(theDelay);
  }
}

void setup() {

  Serial.begin(9600);
  
  Serial.println("Init...");
  initKey();
  
  Serial.println("Trellis Demo");
  // INT pin requires a pullup
  pinMode(INTPIN, INPUT);
  digitalWrite(INTPIN, HIGH);
  
  // begin() with the addresses of each panel in order Top 1st half to Bottom 2nd half
  trellis.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75);  // or four!

  // Blink up all the LEDs in order
  for (uint8_t i=0; i<numKeys; i++) {
    //blinkLed(i, FAST);
  }

  for (uint8_t i=0; i<NB_CHANNELS; i++) {
    trellis.setLED(channelMute[i]);
  }
  trellis.setLED(soundSet[0]);
  trellis.setLED(mood[0]);
  trellis.writeDisplay();
}

void loop() {
  int active = 0;
  int isActive = -1;
  delay(30); // 30ms delay is required, dont remove me!
  // If a button was just pressed or released...
  if (trellis.readSwitches()) {
    // go through every button
    for (uint8_t i=0; i<numKeys; i++) {
      // if it was pressed, turn it on
      if (trellis.justPressed(i)) {
        blinkLed(i);
        Serial.print("Touch #"); Serial.print(i);

        /*
         * Channel group
         */

        //Mute
        if ( in_array(i, channelMute, NB_CHANNELS) ){
          position = in_array(i, channelMute, NB_CHANNELS);
          Serial.print(" Groupe Mute: ");Serial.println(position);
          if (trellis.isLED(i)){
            trellis.clrLED(i);
          }else{
            trellis.setLED(i);
          }

          for (uint8_t y=0; y<NB_CHANNELS; y++) {
            trellis.clrLED(channelSingle[y]);
            if (trellis.isLED(channelMute[y])){
              active++;
              isActive = y;
            }
          }
          if ( active == 1){
            trellis.setLED(channelSingle[isActive]);
          }
          trellis.clrLED(general[0]);
          trellis.writeDisplay();

        //Minus
        }else if( in_array(i, channelMin, NB_CHANNELS) ){
          position = in_array(i, channelMin, NB_CHANNELS);
          Serial.print(" Groupe Minus: ");Serial.println(position);
        
        //Max
        }else if( in_array(i, channelMax, NB_CHANNELS) ){
          position = in_array(i, channelMax, NB_CHANNELS);
          Serial.print(" Groupe Max: ");Serial.println(position);
        
        //Single
        }else if( in_array(i, channelSingle, NB_CHANNELS) ){
          position = in_array(i, channelSingle, NB_CHANNELS);
          Serial.print(" Groupe Single: ");Serial.println(position);
          if (!trellis.isLED(i)){
            for (uint8_t y=0; y<NB_CHANNELS; y++) {
              trellis.clrLED(channelMute[y]);
              trellis.clrLED(channelSingle[y]);
            }
            trellis.setLED(i-3);
            trellis.setLED(i);
            trellis.clrLED(general[0]);
          }
          trellis.writeDisplay();
        
        /*
        * SoundSet Group
        */
        }else if( in_array(i, soundSet, NB_SOUNDSET) ){
          position = in_array(i, soundSet, NB_SOUNDSET);
          Serial.print(" Groupe SoundSet: ");Serial.println(position);
          if (!trellis.isLED(i)){
            for (uint8_t y=0; y<NB_SOUNDSET; y++) {
              trellis.clrLED(soundSet[y]);
            }
            trellis.setLED(i);
          }
          trellis.writeDisplay();

        /*
        * Mood Group
        */
        }else if( in_array(i, mood, NB_MOOD) ){
          position = in_array(i, mood, NB_MOOD);
          Serial.print(" Groupe Mood: ");Serial.println(position);
          if (!trellis.isLED(i)){
            for (uint8_t y=0; y<NB_MOOD; y++) {
              trellis.clrLED(mood[y]);
            }
            trellis.setLED(i);
          }
          trellis.writeDisplay();

        /*
        * MoodSound Group
        */
        }else if( in_array(i, moodSound, NB_MOODSOUND) ){
          position = in_array(i, moodSound, NB_MOODSOUND);
          Serial.print(" Groupe Mood Sound: ");Serial.println(position);

        /*
        * ThemeSound Group
        */
        }else if( in_array(i, themeSound, NB_THEMESOUND) ){
          position = in_array(i, themeSound, NB_THEMESOUND);
          Serial.print(" Groupe Theme Sound: ");Serial.println(position);

        /*
        * General Group
        */
        }else if( in_array(i, general, NB_GENERAL) ){
          position = in_array(i, general, NB_GENERAL);
          Serial.print(" Groupe General: ");Serial.println(position);
          switch (position) {
            case 1: //MuteAll
              if (trellis.isLED(i)){
                trellis.clrLED(i);
                active = 0;
                for (uint8_t y=0; y<NB_CHANNELS; y++) {
                  if (saved[y] == 1){
                    trellis.setLED(channelMute[y]);
                    active++;
                    isActive = y;
                  }
                }
                Serial.print(" channel active: ");Serial.println(active);
                if (active == 1){
                  trellis.setLED(channelSingle[isActive]);
                }
              }else{
                trellis.setLED(i);
                for (uint8_t y=0; y<NB_CHANNELS; y++) {
                  if (trellis.isLED(channelMute[y])){
                    saved[y] = 1;
                  }else{
                    saved[y] = 0;
                  }
                  trellis.clrLED(channelMute[y]);
                  trellis.clrLED(channelSingle[y]);
                }
              }
              trellis.writeDisplay();              
              break;
            case 2: //MinusALL
              break;
            case 3: //MaxAll
              break;
            case 4: //Play/Pause
              break;
          }
        }
      } 
    }
  }
}

