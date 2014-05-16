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

#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
  // create shield-example object!
  //Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

void setup() {
  Serial.begin(9600);
  Serial.println("VS1053 GPIO test");

  // disable the card (we won't be using it)
  pinMode(CARDCS, OUTPUT);
  digitalWrite(CARDCS, HIGH);  
  
  // initialise the music player
  if (!musicPlayer.begin()) {
    Serial.println("VS1053 not found");
    while (1);  // don't do anything more
  }

  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working  
}

void loop() {  
  for (uint8_t i=0; i<8; i++) { 
    musicPlayer.GPIO_pinMode(i, OUTPUT);
    
    musicPlayer.GPIO_digitalWrite(i, HIGH);
    Serial.print("GPIO: "); Serial.println(musicPlayer.GPIO_digitalRead(i));
    musicPlayer.GPIO_digitalWrite(i, LOW);
    Serial.print("GPIO: "); Serial.println(musicPlayer.GPIO_digitalRead(i));

    musicPlayer.GPIO_pinMode(i, INPUT);

    delay(100);  
  }
}