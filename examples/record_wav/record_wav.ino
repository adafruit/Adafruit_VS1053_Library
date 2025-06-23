/***************************************************
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Based on code written by Limor Fried/Ladyada for Adafruit Industries.
  Wav file recording by Ben Hitchcock
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// This is a beta demo of Wav file recording.
// Connect a button between digital 7 on the Arduino and ground,
// press and hold to record.  Once the button is lifted, the recording will play back.

// A mic or line-in connection is required. See page 13 of the
// datasheet for wiring

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
#define RESET 9      // VS1053 reset pin (output)
#define CS 10        // VS1053 chip select pin (output)
#define DCS 8        // VS1053 Data/command select pin (output)
#define CARDCS 4     // Card chip select pin
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

#define REC_BUTTON 7

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(RESET, CS, DCS, DREQ, CARDCS);

File recording;  // the file we will save our recording to
#define RECBUFFSIZE 128  // 64 or 128 bytes.
uint8_t recording_buffer[RECBUFFSIZE];

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Wav Recording Test");

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
  musicPlayer.setVolume(10,10);

  // when the button is pressed, record!
  pinMode(REC_BUTTON, INPUT);
  digitalWrite(REC_BUTTON, HIGH);

   musicPlayer.softReset();
}

uint8_t isRecording = false;
uint16_t bytesWritten = 0;
uint16_t sampleRate = 8000;

void loop() {
  char filename[15];

  if (!isRecording && !digitalRead(REC_BUTTON)) {
    Serial.println("Begin recording");
    isRecording = true;

    // Check if the file exists already
    strcpy(filename, "RECORD00.WAV");
    for (uint8_t i = 0; i < 100; i++) {
      filename[6] = '0' + i/10;
      filename[7] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(filename)) {
        break;
      }
    }
    Serial.print("Recording to "); Serial.println(filename);

    createWavTemplate(filename, sampleRate);

    recording = SD.open(filename, FILE_WRITE);
    if (! recording) {
       Serial.println("Couldn't open file to record!");
       while (1);
    }
    musicPlayer.startRecordWav(true, sampleRate); // use microphone (for linein, pass in 'false')
  }

  if (isRecording)
    saveRecordedData(isRecording);

  if (isRecording && digitalRead(REC_BUTTON)) {

    Serial.println("End recording");

    isRecording = false;
    // flush all the data!
    bytesWritten = saveRecordedData(isRecording);

    // Note: We need to read a full block before telling the VS1053 to stop recording.
    musicPlayer.stopRecordWav();

    // close it up
    recording.close();

    finalizeWavTemplate(filename);

    Serial.print(bytesWritten); Serial.println(" Bytes Written");

    musicPlayer.softReset();

    Serial.print("Playing track "); Serial.println(filename);

    musicPlayer.playFullFile(filename);
    Serial.println("Finished Playing track");
  }
}

uint16_t saveRecordedData(boolean isrecord) {
  uint16_t written = 0;

  // read how many words are waiting for us
  uint16_t wordswaiting = musicPlayer.recordedWordsWaiting();

  // try to process 128 words (256 bytes) at a time, for best speed
  while (wordswaiting > 128) {
    //Serial.print("Waiting: "); Serial.println(wordswaiting);
    // for example 128 bytes x 4 loops = 512 bytes
    for (int x=0; x < 256/RECBUFFSIZE; x++) {
      // fill the buffer!
      for (uint16_t addr=0; addr < (RECBUFFSIZE); addr+=2) {
        uint16_t t = musicPlayer.recordedReadWord();
        //Serial.println(t, HEX);
        recording_buffer[addr] = highByte(t);
        recording_buffer[addr+1] = lowByte(t);
      }
      if (! recording.write(recording_buffer, RECBUFFSIZE)) {
            Serial.print("Couldn't write "); Serial.println(RECBUFFSIZE);
            while (1);
      }
    }
    // flush 256 bytes at a time
    recording.flush();
    written += 128;
    wordswaiting -= 128;
  }

  if (!isrecord) {

    // The recorder prefers us to stop at the block boundary (128 words for mono, 256 words for stereo).
    // Hence we have to wait a bit for the recorder to fill up the block, and read that entire block
    // before continuing.
    // Here we take note of how many words are waiting when the user let go of the button, and we
    // pause until there is a full block able to be read.

    wordswaiting = musicPlayer.recordedWordsWaiting();
    uint16_t wordsToSave = wordswaiting;

    // Pause until a full block is waiting to be read.
    while(wordswaiting < 128){
      wordswaiting = musicPlayer.recordedWordsWaiting();
    }

    // Read a full block (128 words for mono)
    uint16_t addr = 0;
    for (int x=0; x < 128; x++) {
      // Read a sample from the recorder
      uint16_t t = musicPlayer.recordedReadWord();

      // If we're reading samples from before the button was released, then write them to the buffer
      if(x < wordsToSave){
        recording_buffer[addr] = highByte(t);
        recording_buffer[addr+1] = lowByte(t);
        addr += 2;

        // If the buffer is full, save it
        if (addr >= RECBUFFSIZE) {
          if (!recording.write(recording_buffer, addr)) {
            Serial.println("Couldn't write!"); while (1);
          }
          written += addr;
          addr = 0;
        }
      }
    }

    // Save any remaining samples
    if (addr != 0) {
      if (!recording.write(recording_buffer, addr)) {
        Serial.println("Couldn't write!"); while (1);
      }
      written += addr;
    }

    recording.flush();
  }

  return written;
}

// Create a WAV file header
void createWavTemplate(const char* filename, unsigned int sampleRate){

  if(SD.exists(filename)){
    SD.remove(filename);
  }

  File sFile = SD.open(filename, FILE_WRITE);
  if(!sFile){
      return;
  } else {

    sFile.seek(0);
    sFile.write((byte*)"RIFF    WAVEfmt ", 16); // Note: the data after "RIFF" will be overwritten with the file size

    // NOTE: Little endian!  The order of the data bytes is reversed.

    // Chunk size: 0 0 0 20, format code 0x11 for IMA ADPCM, number channels 0 1, sampleRate (4 bytes)
    byte data[] = {20,0,0,0, 0x11,0, 1,0, lowByte(sampleRate),highByte(sampleRate), 0, 0};
    sFile.write((byte*)data,12);

    unsigned int byteRate = 0xfd7; // 4 bit ADPCM 8 kHz Mono
    data[0] = lowByte(byteRate); data[1] = highByte(byteRate); data[2] = 0; data[3] = 0;   // Data rate: Sample rate * bytes per sample

    data[4] = 0; data[5] = 1; // BlockAlign, in this case it is 16 (0x01 0x00)

    data[6] = 0x04; data[7] = 0;  // Bits per sample, in this case 4-bit ADPCM

    data[8] = 0x02; data[9] = 0;  // Byte Extra Data

    data[10] = 0xf9; data[11] = 0x01;  // Samples per block (505)

    sFile.write((byte*)data, 12);

    sFile.write((byte*)"fact", 4); // SubChunk2ID

    data[0] = 0x04; data[1] = 0x00; data[2] = 0x00; data[3] = 0x00;   // SubChunk2Size - in this case 4 bytes

    data[4] = 0x00; data[5] = 0x00; data[6] = 0x00; data[7] = 0x00;   // NumOfSamples - will be overwritten when finalizing the wav header
    sFile.write((byte*)data,8);

    sFile.write((byte*)"data    ", 8);  // Start of data portion of the file

    sFile.close();
  }
}

void finalizeWavTemplate(const char* filename){
  unsigned long fSize = 0;

  #ifdef FILE_APPEND
  File sFile = SD.open(filename, FILE_WRITE);
  #else
  File sFile = SD.open(filename, O_READ | O_WRITE);
  #endif

  if(!sFile){
    return;
  }
  fSize = sFile.size() - 8;

  // Set the file size header after the RIFF placeholder
  sFile.seek(4); byte data[4] = {lowByte(fSize),highByte(fSize), (byte)(fSize >> 16), (byte)(fSize >> 24)};
  sFile.write(data, 4);

  // Set the data size header (bytes)
  sFile.seek(56);
  fSize = fSize - 52;
  data[0] = lowByte(fSize); data[1]=highByte(fSize); data[2]=(byte)(fSize >> 16); data[3]= (byte)(fSize >> 24);
  sFile.write((byte*)data,4);

  // Set the number of samples header (for IMA ADPCM this is: numBlocks * 505)
  unsigned long numSamples = fSize / 256 * 505;  // For IMA ADPCM there are 256 bytes to a block.
  data[0] = lowByte(numSamples); data[1] = highByte(numSamples); data[2] = (byte)(numSamples >> 16); data[3] = (byte)(numSamples >> 24);
  sFile.seek(48);
  sFile.write((byte*)data,4);

  sFile.close();
}
