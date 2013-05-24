/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
#define RESET 9      // VS1053 reset pin (output)
#define CS 10        // VS1053 chip select pin (output)
#define DCS 8        // VS1053 Data/command select pin (output)
#define DREQ 3       // VS1053 Data request pin (into Arduino)
#define CARDCS 4     // Card chip select pin


Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(RESET, CS, DCS, DREQ, CARDCS);
// Alternately, use 'soft SPI'. Requires Adafruit's flexible SD library
// Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(MOSI, MISO, CLK, RESET, CS, DCS, DREQ, CARDCS);

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Library Test");

  // initialise the music player
  if (!musicPlayer.begin()) {
    Serial.println("VS1053 not found");
    while (1);  // don't do anything more
  }

  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
 
  if (!SD.begin(CARDCS)) {
    Serial.println("SD failed, or not present");
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20,20);

  /***** Two interrupt options! *******/ 
  // This option uses timer0, this means timer1 & t2 are not required
  // (so you can use 'em for Servos, etc) BUT millis() can lose time
  // since we're hitchhiking on top of the millis() tracker
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
  
  // This option uses a pin interrupt. No timers required! But DREQ
  // must be on an interrupt pin. For Uno/Duemilanove/Diecimilla
  // that's Digital #2 or #3
  // See http://arduino.cc/en/Reference/attachInterrupt for other pins
  // *** This method is preferred
  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    Serial.println("DREQ pin is not an interrupt pin");
}

void loop() {  
  // Alternately, we can just play an entire file at once
  // This doesn't happen in the background, instead, the entire
  // file is played and the program will continue when it's done!
  musicPlayer.playFullFile("track001.ogg");

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile("track001.mp3")) {
    Serial.print("Could not open file");
    return;
  }
  Serial.println("Started playing");

  while (musicPlayer.playingMusic) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Done playing music");
}

