#include <Adafruit_MP3.h>

static volatile uint8_t *clkportreg, *misoportreg, *mosiportreg;
static uint8_t clkpin, misopin, mosipin;

Adafruit_MP3::Adafruit_MP3(uint8_t mosi, uint8_t miso, uint8_t clk, 
			   uint8_t rst, uint8_t cs, uint8_t dcs, uint8_t dreq) {
  _mosi = mosi;
  _miso = miso;
  _clk = clk;
  _reset = rst;
  _cs = cs;
  _dcs = dcs;
  _dreq = dreq;


  clkportreg = portOutputRegister(digitalPinToPort(_clk));
  clkpin = digitalPinToBitMask(_clk);
  misoportreg = portInputRegister(digitalPinToPort(_miso));
  misopin = digitalPinToBitMask(_miso);
  mosiportreg = portOutputRegister(digitalPinToPort(_mosi));
  mosipin = digitalPinToBitMask(_mosi);
}

uint16_t Adafruit_MP3::decodeTime() {
  return sci_read(VS1053_REG_DECODETIME);
}


void Adafruit_MP3::reset() {
  // TODO: http://www.vlsi.fi/player_vs1011_1002_1003/modularplayer/vs10xx_8c.html#a3
  // hardware reset
  digitalWrite(_reset, LOW);
  delay(100);
  digitalWrite(_cs, HIGH);
  digitalWrite(_dcs, HIGH);
  digitalWrite(_reset, HIGH);
  delay(100);

  sci_write(VS1053_REG_CLOCKF, 0x6000);
}

uint8_t Adafruit_MP3::begin(void) {

  pinMode(_reset, OUTPUT);
  digitalWrite(_reset, LOW);
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
  pinMode(_dcs, OUTPUT);
  digitalWrite(_dcs, HIGH);
  pinMode(_mosi, OUTPUT);
  pinMode(_clk, OUTPUT);
  pinMode(_miso, INPUT);
  pinMode(_dreq, INPUT);

  reset();

  return (sci_read(VS1053_REG_STATUS) >> 4) & 0x0F;
}

void Adafruit_MP3::dumpRegs(void) {
  Serial.print("Mode = 0x"); Serial.println(sci_read(VS1053_REG_MODE), HEX);
  Serial.print("Stat = 0x"); Serial.println(sci_read(VS1053_REG_STATUS), HEX);
  Serial.print("ClkF = 0x"); Serial.println(sci_read(VS1053_REG_CLOCKF), HEX);
  Serial.print("Vol. = 0x"); Serial.println(sci_read(VS1053_REG_VOLUME), HEX);
}



uint16_t Adafruit_MP3::sci_read(uint8_t addr) {
  uint16_t data;

  digitalWrite(_cs, LOW);  
  spiwrite(VS1053_SCI_READ);
  spiwrite(addr);
  delayMicroseconds(10);
  data = spiread();
  data <<= 8;
  data |= spiread();
  digitalWrite(_cs, HIGH);

  return data;
}


void Adafruit_MP3::sci_write(uint8_t addr, uint16_t data) {
  digitalWrite(_cs, LOW);  
  spiwrite(VS1053_SCI_WRITE);
  spiwrite(addr);
  spiwrite(data >> 8);
  spiwrite(data & 0xFF);
  digitalWrite(_cs, HIGH);
}



uint8_t Adafruit_MP3::spiread(void)
{
  int8_t i, x;
  x = 0;

  // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low

  for (i=7; i>=0; i--) {
    if ((*misoportreg) & misopin)
      x |= (1<<i);    
    *clkportreg |= clkpin;
    *clkportreg &= ~clkpin;
//    asm("nop; nop");
  }
  // Make sure clock ends low
  *clkportreg &= ~clkpin;

  return x;
}

void Adafruit_MP3::spiwrite(uint8_t c)
{
  // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low

  for (int8_t i=7; i>=0; i--) {
    *clkportreg &= ~clkpin;
    if (c & (1<<i)) {
      *mosiportreg |= mosipin;
    } else {
      *mosiportreg &= ~mosipin;
    }
    *clkportreg |= clkpin;
  }
  *clkportreg &= ~clkpin;   // Make sure clock ends low
}



void Adafruit_MP3::sineTest(uint8_t n, uint16_t ms) {
  reset();
  
  uint16_t mode = sci_read(VS1053_REG_MODE);
  mode |= 0x0020;
  sci_write(VS1053_REG_MODE, mode);

  while (!digitalRead(_dreq));
	 //  delay(10);

  digitalWrite(_dcs, LOW);  
  spiwrite(0x53);
  spiwrite(0xEF);
  spiwrite(0x6E);
  spiwrite(n);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  digitalWrite(_dcs, HIGH);  
  
  delay(ms);

  digitalWrite(_dcs, LOW);  
  spiwrite(0x45);
  spiwrite(0x78);
  spiwrite(0x69);
  spiwrite(0x74);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  digitalWrite(_dcs, HIGH);  
}
