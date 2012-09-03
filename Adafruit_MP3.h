#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h> 

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

#define VS1053_MODE_SM_DIFF 0x0001
#define VS1053_MODE_SM_LAYER12 0x0002
#define VS1053_MODE_SM_RESET 0x0004
#define VS1053_MODE_SM_CANCEL 0x0008
#define VS1053_MODE_SM_EARSPKLO 0x0010
#define VS1053_MODE_SM_TESTS 0x0020
#define VS1053_MODE_SM_STREAM 0x0040


class Adafruit_MP3 {
 public:
  Adafruit_MP3(uint8_t mosi, uint8_t miso, uint8_t clk, 
	       uint8_t rst, uint8_t cs, uint8_t dcs, uint8_t dreq);
  uint8_t begin(void);
  void reset(void);
  void dumpRegs(void);
  uint16_t sci_read(uint8_t addr);
  void sci_write(uint8_t addr, uint16_t data);
  void sineTest(uint8_t n, uint16_t ms);
  void spiwrite(uint8_t d);
  uint8_t spiread(void);
  uint16_t decodeTime();

 private:
  uint8_t _mosi, _miso, _clk, _reset, _cs, _dcs, _dreq;


};
