/***************************************************
  This is a library for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#ifndef ADAFRUIT_VS1053_H
#define ADAFRUIT_VS1053_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#if !defined(ARDUINO_STM32_FEATHER)
#include "pins_arduino.h"
#include "wiring_private.h"
#endif

#include <SPI.h>
#if defined(PREFER_SDFAT_LIBRARY)
 #include <SdFat.h>
 extern SdFat SD;
#else
 #include <SD.h>
#endif

// define here the size of a register!
#if defined(ARDUINO_STM32_FEATHER)
  typedef volatile uint32 RwReg;
  typedef uint32_t PortMask;
#elif defined(ARDUINO_ARCH_AVR)
  typedef volatile uint8_t RwReg;
  typedef uint8_t PortMask;
#elif defined (__arm__)
  #if defined(TEENSYDUINO)
  typedef volatile uint8_t RwReg;
  typedef uint8_t PortMask;
  #else
  typedef volatile uint32_t RwReg;
  typedef uint32_t PortMask;
  #endif
#elif defined (ESP8266) || defined (ESP32)
  typedef volatile uint32_t RwReg;
  typedef uint32_t PortMask;
#elif defined (__ARDUINO_ARC__)
  typedef volatile uint32_t RwReg;
  typedef uint32_t PortMask;
#else
  typedef volatile uint8_t RwReg;
  typedef uint8_t PortMask;
#endif

typedef volatile RwReg PortReg;

#define VS1053_FILEPLAYER_TIMER0_INT 255 // allows useInterrupt to accept pins 0 to 254
#define VS1053_FILEPLAYER_PIN_INT 5

#define VS1053_SCI_READ 0x03
#define VS1053_SCI_WRITE 0x02

#define VS1053_REG_MODE  0x00
#define VS1053_REG_STATUS 0x01
#define VS1053_REG_BASS 0x02
#define VS1053_REG_CLOCKF 0x03
#define VS1053_REG_DECODETIME 0x04
#define VS1053_REG_AUDATA 0x05
#define VS1053_REG_WRAM 0x06
#define VS1053_REG_WRAMADDR 0x07
#define VS1053_REG_HDAT0 0x08
#define VS1053_REG_HDAT1 0x09
#define VS1053_REG_VOLUME 0x0B

#define VS1053_GPIO_DDR 0xC017
#define VS1053_GPIO_IDATA 0xC018
#define VS1053_GPIO_ODATA 0xC019

#define VS1053_INT_ENABLE  0xC01A

#define VS1053_MODE_SM_DIFF 0x0001
#define VS1053_MODE_SM_LAYER12 0x0002
#define VS1053_MODE_SM_RESET 0x0004
#define VS1053_MODE_SM_CANCEL 0x0008
#define VS1053_MODE_SM_EARSPKLO 0x0010
#define VS1053_MODE_SM_TESTS 0x0020
#define VS1053_MODE_SM_STREAM 0x0040
#define VS1053_MODE_SM_SDINEW 0x0800
#define VS1053_MODE_SM_ADPCM 0x1000
#define VS1053_MODE_SM_LINE1 0x4000
#define VS1053_MODE_SM_CLKRANGE 0x8000


#define VS1053_SCI_AIADDR 0x0A
#define VS1053_SCI_AICTRL0 0x0C
#define VS1053_SCI_AICTRL1 0x0D
#define VS1053_SCI_AICTRL2 0x0E
#define VS1053_SCI_AICTRL3 0x0F

#define VS1053_DATABUFFERLEN 32


class Adafruit_VS1053 {
 public:
  Adafruit_VS1053(int8_t mosi, int8_t miso, int8_t clk,
		  int8_t rst, int8_t cs, int8_t dcs, int8_t dreq);
  Adafruit_VS1053(int8_t rst, int8_t cs, int8_t dcs, int8_t dreq);
  uint8_t begin(void);
  void reset(void);
  void softReset(void);
  uint16_t sciRead(uint8_t addr);
  void sciWrite(uint8_t addr, uint16_t data);
  void sineTest(uint8_t n, uint16_t ms);
  void spiwrite(uint8_t d);
  void spiwrite(uint8_t *c, uint16_t num);
  uint8_t spiread(void);

  uint16_t decodeTime(void);
  void setVolume(uint8_t left, uint8_t right);
  void dumpRegs(void);

  void playData(uint8_t *buffer, uint8_t buffsiz);
  boolean readyForData(void);
  void applyPatch(const uint16_t *patch, uint16_t patchsize);
  uint16_t loadPlugin(char *fn);

  void GPIO_digitalWrite(uint8_t i, uint8_t val);
  void GPIO_digitalWrite(uint8_t i);
  uint16_t GPIO_digitalRead(void);
  boolean GPIO_digitalRead(uint8_t i);
  void GPIO_pinMode(uint8_t i, uint8_t dir);

  boolean prepareRecordOgg(char *plugin);
  void startRecordOgg(boolean mic);
  void stopRecordOgg(void);
  uint16_t recordedWordsWaiting(void);
  uint16_t recordedReadWord(void);

  uint8_t mp3buffer[VS1053_DATABUFFERLEN];

#ifdef ARDUINO_ARCH_SAMD
protected:
  uint32_t  _dreq;
private:
  int32_t _mosi, _miso, _clk, _reset, _cs, _dcs;
  boolean useHardwareSPI;
#else
 protected:
  uint8_t  _dreq;
 private:
  int8_t _mosi, _miso, _clk, _reset, _cs, _dcs;
  boolean useHardwareSPI;
#endif
};


class Adafruit_VS1053_FilePlayer : public Adafruit_VS1053 {
 public:
  Adafruit_VS1053_FilePlayer (int8_t mosi, int8_t miso, int8_t clk,
			      int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
			      int8_t cardCS);
  Adafruit_VS1053_FilePlayer (int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
			      int8_t cardCS);
  Adafruit_VS1053_FilePlayer (int8_t cs, int8_t dcs, int8_t dreq,
			      int8_t cardCS);

  boolean begin(void);
  boolean useInterrupt(uint8_t type);
  File currentTrack;
  volatile boolean playingMusic;
  void feedBuffer(void);
  static boolean isMP3File(const char* fileName);
  unsigned long mp3_ID3Jumper(File mp3);
  boolean startPlayingFile(const char *trackname);
  boolean playFullFile(const char *trackname);
  void stopPlaying(void);
  boolean paused(void);
  boolean stopped(void);
  void pausePlaying(boolean pause);

 private:
  void feedBuffer_noLock(void);

  uint8_t _cardCS;
};

#endif // ADAFRUIT_VS1053_H
