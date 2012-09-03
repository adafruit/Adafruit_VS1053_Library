#include <SPI.h>
#include <Adafruit_MP3.h>
#include <SD.h>

#define MOSI 11
#define MISO 12
#define CLK 13
#define RESET 9
#define CS 10
#define DCS 8
#define CARDCS 4
#define DREQ 2

#define MP3DATABUFFERLEN 32
uint8_t mp3buffer[MP3DATABUFFERLEN];

Adafruit_MP3 mp3 = Adafruit_MP3(MOSI, MISO, CLK, RESET, CS, DCS, DREQ);

/************* HARDWARE SPI ENABLE/DISABLE */
int8_t saved_spimode;

void disableSPI(void) {
  saved_spimode = SPCR;
  SPCR = 0;
}

void enableSPI(void) {
  SPCR = saved_spimode; 
}

void setup() {
  Serial.begin(9600);
  Serial.println("VS1053 test");

  pinMode(CARDCS, OUTPUT);
  digitalWrite(CARDCS, HIGH);  
  uint8_t v  = mp3.begin();   
  mp3.dumpRegs();
  Serial.print("Version = "); Serial.println(v);
  if (v != 4) {
    Serial.println("VS1053 not found");
    while (1);
  }
  mp3.sineTest(0x44, 500);
  // start again!
  mp3.begin();
  
  if (!SD.begin(4)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  disableSPI();
  
  mp3.dumpRegs();
  
  playFullMP3("track001.mp3");
}

File track;


void playFullMP3(char *trackname) {  
  boolean need_data = true;

  enableSPI();
  track = SD.open(trackname);
  disableSPI();
  if (!track) {
    Serial.print("Could not open ");
    Serial.println(trackname);
    return;
  }
  Serial.println("Found file!");

  uint16_t lasttime = millis() % 1000;
  
  while (1) {
    // prefill buffer!
    enableSPI();
    int ret = track.read(mp3buffer, MP3DATABUFFERLEN);
    disableSPI();
    if (ret != MP3DATABUFFERLEN) {
      Serial.print(ret);
      Serial.println("-> done");
      break;
    }
    // TODO: take care of under 32 bytes remaining!

    while (!digitalRead(DREQ)) {
       // chillax 
    }
    // DREQ high! send it some data :)
    
    digitalWrite(DCS, LOW);
    for (uint8_t i=0; i<MP3DATABUFFERLEN; i++) {
       mp3.spiwrite(mp3buffer[i]);
    }
    digitalWrite(DCS, HIGH);
    if (lasttime >  (millis() % 1000)) {
      uint16_t t = mp3.decodeTime();
      Serial.print(t/60); Serial.print(':');
      if (t % 60 < 10) Serial.print('0');
      Serial.println(t%60);
    }
    lasttime = millis() % 1000;
  }
  while (!digitalRead(DREQ));
  Serial.println("complete!");
}

void loop() {  
  delay(500);  
}