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
