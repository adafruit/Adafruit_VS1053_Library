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

#include <Adafruit_VS1053.h>
#include <SD.h>

#if defined(ARDUINO_STM32_FEATHER)
   #define digitalPinToInterrupt(x) x
#endif

static Adafruit_VS1053_FilePlayer *myself;

#ifndef _BV
  #define _BV(x) (1<<(x))
#endif

#if defined(__AVR__)
SIGNAL(TIMER0_COMPA_vect) {
  myself->feedBuffer();
}
#endif

volatile boolean feedBufferSem = false;

static void feeder(void) {
  if (feedBufferSem) return;

  myself->feedBuffer();
}

#define VS1053_CONTROL_SPI_SETTING  SPISettings(250000,  MSBFIRST, SPI_MODE0)
#define VS1053_DATA_SPI_SETTING     SPISettings(8000000, MSBFIRST, SPI_MODE0)


boolean Adafruit_VS1053_FilePlayer::useInterrupt(uint8_t type) {
  myself = this;  // oy vey
    
  if (type == VS1053_FILEPLAYER_TIMER0_INT) {
#if defined(__AVR__)
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    return true;
#elif defined(__arm__) && defined(CORE_TEENSY)
    IntervalTimer *t = new IntervalTimer();
    return (t && t->begin(feeder, 1024)) ? true : false;
#elif defined(ARDUINO_STM32_FEATHER)
    HardwareTimer timer(3);
    // Pause the timer while we're configuring it
    timer.pause();

    // Set up period
    timer.setPeriod(25000); // in microseconds

    // Set up an interrupt on channel 1
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
    timer.attachCompare1Interrupt(feeder); 

    // Refresh the timer's count, prescale, and overflow
    timer.refresh();

    // Start the timer counting
    timer.resume();
    
#else
    return false;
#endif
  }
  if (type == VS1053_FILEPLAYER_PIN_INT) {
    int8_t irq = digitalPinToInterrupt(_dreq);
    //Serial.print("Using IRQ "); Serial.println(irq);
    if (irq == -1) 
      return false;
#if defined(SPI_HAS_TRANSACTION) && !defined(ESP8266) && !defined(ESP32) && !defined(ARDUINO_STM32_FEATHER)
    SPI.usingInterrupt(irq);
#endif
    attachInterrupt(irq, feeder, CHANGE);
    return true;
  }
  return false;
}

Adafruit_VS1053_FilePlayer::Adafruit_VS1053_FilePlayer(
	       int8_t rst, int8_t cs, int8_t dcs, int8_t dreq, 
	       int8_t cardcs) 
               : Adafruit_VS1053(rst, cs, dcs, dreq) {

  playingMusic = false;

  // Set the card to be disabled while we get the VS1053 up
  pinMode(_cardCS, OUTPUT);
  digitalWrite(_cardCS, HIGH);  
}

Adafruit_VS1053_FilePlayer::Adafruit_VS1053_FilePlayer(
	       int8_t cs, int8_t dcs, int8_t dreq, 
	       int8_t cardcs) 
  : Adafruit_VS1053(-1, cs, dcs, dreq) {

  playingMusic = false;

  // Set the card to be disabled while we get the VS1053 up
  pinMode(_cardCS, OUTPUT);
  digitalWrite(_cardCS, HIGH);  
}


Adafruit_VS1053_FilePlayer::Adafruit_VS1053_FilePlayer(
               int8_t mosi, int8_t miso, int8_t clk, 
	       int8_t rst, int8_t cs, int8_t dcs, int8_t dreq, 
	       int8_t cardcs) 
               : Adafruit_VS1053(mosi, miso, clk, rst, cs, dcs, dreq) {

  playingMusic = false;

  // Set the card to be disabled while we get the VS1053 up
  pinMode(_cardCS, OUTPUT);
  digitalWrite(_cardCS, HIGH);  
}

boolean Adafruit_VS1053_FilePlayer::begin(void) {
  uint8_t v  = Adafruit_VS1053::begin();   

  //dumpRegs();
  //Serial.print("Version = "); Serial.println(v);
  return (v == 4);
}


boolean Adafruit_VS1053_FilePlayer::playFullFile(const char *trackname) {
  if (! startPlayingFile(trackname)) return false;

  while (playingMusic) {
    // twiddle thumbs
    noInterrupts();
    feedBuffer();
    interrupts();
    delay(100);           // give IRQs a chance
  }
  // music file finished!
  return true;
}

void Adafruit_VS1053_FilePlayer::stopPlaying(void) {
  // cancel all playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_CANCEL);
  
  // wrap it up!
  playingMusic = false;
  currentTrack.close();
}

void Adafruit_VS1053_FilePlayer::pausePlaying(boolean pause) {
  if (pause) 
    playingMusic = false;
  else {
    playingMusic = true;
    feedBuffer();
  }
}

boolean Adafruit_VS1053_FilePlayer::paused(void) {
  return (!playingMusic && currentTrack);
}

boolean Adafruit_VS1053_FilePlayer::stopped(void) {
  return (!playingMusic && !currentTrack);
}


boolean Adafruit_VS1053_FilePlayer::startPlayingFile(const char *trackname) {
  // reset playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
  // resync
  sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
  sciWrite(VS1053_REG_WRAM, 0);

  currentTrack = SD.open(trackname);
  if (!currentTrack) {
    return false;
  }

  // don't let the IRQ get triggered by accident here
  noInterrupts();

  // As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0
  sciWrite(VS1053_REG_DECODETIME, 0x00);
  sciWrite(VS1053_REG_DECODETIME, 0x00);

  playingMusic = true;

  // wait till its ready for data
  while (! readyForData() );

  // fill it up!
  while (playingMusic && readyForData()) {
    feedBuffer();
  }
  
  // ok going forward, we can use the IRQ
  interrupts();

  return true;
}

void Adafruit_VS1053_FilePlayer::feedBuffer(void) {
  // dont run twice in case interrupts collided
  if (feedBufferSem) return;

  feedBufferSem = true;

  if ((! playingMusic) // paused or stopped
      || (! currentTrack) 
      || (! readyForData())) {
    feedBufferSem = false;
    return; // paused or stopped
  }

  // Feed the hungry buffer! :)
  while (readyForData()) {
    // Read some audio data from the SD card file
    int bytesread = currentTrack.read(mp3buffer, VS1053_DATABUFFERLEN);
    
    if (bytesread == 0) {
      // must be at the end of the file, wrap it up!
      playingMusic = false;
      currentTrack.close();
      break;
    }

    playData(mp3buffer, bytesread);
  }
  feedBufferSem = false;
  return;
}


/***************************************************************/

/* VS1053 'low level' interface */
static volatile PortReg *clkportreg, *misoportreg, *mosiportreg;
static PortMask clkpin, misopin, mosipin;

Adafruit_VS1053::Adafruit_VS1053(int8_t mosi, int8_t miso, int8_t clk, 
			   int8_t rst, int8_t cs, int8_t dcs, int8_t dreq) {
  _mosi = mosi;
  _miso = miso;
  _clk = clk;
  _reset = rst;
  _cs = cs;
  _dcs = dcs;
  _dreq = dreq;

  useHardwareSPI = false;

  clkportreg = portOutputRegister(digitalPinToPort(_clk));
  clkpin = digitalPinToBitMask(_clk);
  misoportreg = portInputRegister(digitalPinToPort(_miso));
  misopin = digitalPinToBitMask(_miso);
  mosiportreg = portOutputRegister(digitalPinToPort(_mosi));
  mosipin = digitalPinToBitMask(_mosi);
}


Adafruit_VS1053::Adafruit_VS1053(int8_t rst, int8_t cs, int8_t dcs, int8_t dreq) {
  _mosi = 0;
  _miso = 0;
  _clk = 0;
  useHardwareSPI = true;
  _reset = rst;
  _cs = cs;
  _dcs = dcs;
  _dreq = dreq;
}


void Adafruit_VS1053::applyPatch(const uint16_t *patch, uint16_t patchsize) {
  uint16_t i = 0;

 // Serial.print("Patch size: "); Serial.println(patchsize);
  while ( i < patchsize ) {
    uint16_t addr, n, val;

    addr = pgm_read_word(patch++);
    n = pgm_read_word(patch++);
    i += 2;

    //Serial.println(addr, HEX);
    if (n & 0x8000U) { // RLE run, replicate n samples 
      n &= 0x7FFF;
      val = pgm_read_word(patch++);
      i++;
      while (n--) {
	sciWrite(addr, val);
      }      
    } else {           // Copy run, copy n samples 
      while (n--) {
	val = pgm_read_word(patch++);
	i++;
	sciWrite(addr, val);
      }
    }
  }
}


uint16_t Adafruit_VS1053::loadPlugin(char *plugname) {

  File plugin = SD.open(plugname);
  if (!plugin) {
    Serial.println("Couldn't open the plugin file");
    Serial.println(plugin);
    return 0xFFFF;
  }

  if ((plugin.read() != 'P') ||
      (plugin.read() != '&') ||
      (plugin.read() != 'H'))
    return 0xFFFF;

  uint16_t type;

 // Serial.print("Patch size: "); Serial.println(patchsize);
  while ((type = plugin.read()) >= 0) {
    uint16_t offsets[] = {0x8000UL, 0x0, 0x4000UL};
    uint16_t addr, len;

    //Serial.print("type: "); Serial.println(type, HEX);

    if (type >= 4) {
        plugin.close();
	return 0xFFFF;
    }

    len = plugin.read();    len <<= 8;
    len |= plugin.read() & ~1;
    addr = plugin.read();    addr <<= 8;
    addr |= plugin.read();
    //Serial.print("len: "); Serial.print(len); 
    //Serial.print(" addr: $"); Serial.println(addr, HEX);

    if (type == 3) {
      // execute rec!
      plugin.close();
      return addr;
    }

    // set address
    sciWrite(VS1053_REG_WRAMADDR, addr + offsets[type]);
    // write data
    do {
      uint16_t data;
      data = plugin.read();    data <<= 8;
      data |= plugin.read();
      sciWrite(VS1053_REG_WRAM, data);
    } while ((len -=2));
  }

  plugin.close();
  return 0xFFFF;
}




boolean Adafruit_VS1053::readyForData(void) {
  return digitalRead(_dreq);
}

void Adafruit_VS1053::playData(uint8_t *buffer, uint8_t buffsiz) {
  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
  #endif
  digitalWrite(_dcs, LOW);
  
  spiwrite(buffer, buffsiz);

  digitalWrite(_dcs, HIGH);
  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.endTransaction();
  #endif
}

void Adafruit_VS1053::setVolume(uint8_t left, uint8_t right) {
  uint16_t v;
  v = left;
  v <<= 8;
  v |= right;

  noInterrupts(); //cli();
  sciWrite(VS1053_REG_VOLUME, v);
  interrupts();  //sei();
}

uint16_t Adafruit_VS1053::decodeTime() {
  noInterrupts(); //cli();
  uint16_t t = sciRead(VS1053_REG_DECODETIME);
  interrupts(); //sei();
  return t;
}


void Adafruit_VS1053::softReset(void) {
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);
  delay(100);
}

void Adafruit_VS1053::reset() {
  // TODO: http://www.vlsi.fi/player_vs1011_1002_1003/modularplayer/vs10xx_8c.html#a3
  // hardware reset
  if (_reset >= 0) {
    digitalWrite(_reset, LOW);
    delay(100);
    digitalWrite(_reset, HIGH);
  }
  digitalWrite(_cs, HIGH);
  digitalWrite(_dcs, HIGH);
  delay(100);
  softReset();
  delay(100);

  sciWrite(VS1053_REG_CLOCKF, 0x6000);

  setVolume(40, 40);
}

uint8_t Adafruit_VS1053::begin(void) {
  if (_reset >= 0) {
    pinMode(_reset, OUTPUT);
    digitalWrite(_reset, LOW);
  }

  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
  pinMode(_dcs, OUTPUT);
  digitalWrite(_dcs, HIGH);
  pinMode(_dreq, INPUT);

  if (! useHardwareSPI) {
    pinMode(_mosi, OUTPUT);
    pinMode(_clk, OUTPUT);
    pinMode(_miso, INPUT);
  } else {
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV128); 
  }

  reset();

  return (sciRead(VS1053_REG_STATUS) >> 4) & 0x0F;
}

void Adafruit_VS1053::dumpRegs(void) {
  Serial.print("Mode = 0x"); Serial.println(sciRead(VS1053_REG_MODE), HEX);
  Serial.print("Stat = 0x"); Serial.println(sciRead(VS1053_REG_STATUS), HEX);
  Serial.print("ClkF = 0x"); Serial.println(sciRead(VS1053_REG_CLOCKF), HEX);
  Serial.print("Vol. = 0x"); Serial.println(sciRead(VS1053_REG_VOLUME), HEX);
}


uint16_t Adafruit_VS1053::recordedWordsWaiting(void) {
  return sciRead(VS1053_REG_HDAT1);
}

uint16_t Adafruit_VS1053::recordedReadWord(void) {
  return sciRead(VS1053_REG_HDAT0);
}


boolean Adafruit_VS1053::prepareRecordOgg(char *plugname) {
  sciWrite(VS1053_REG_CLOCKF, 0xC000);  // set max clock
  delay(1);    while (! readyForData() );

  sciWrite(VS1053_REG_BASS, 0);  // clear Bass
  
  softReset();
  delay(1);    while (! readyForData() );

  sciWrite(VS1053_SCI_AIADDR, 0);
  // disable all interrupts except SCI
  sciWrite(VS1053_REG_WRAMADDR, VS1053_INT_ENABLE);
  sciWrite(VS1053_REG_WRAM, 0x02);

  int pluginStartAddr = loadPlugin(plugname);
  if (pluginStartAddr == 0xFFFF) return false;
  Serial.print("Plugin at $"); Serial.println(pluginStartAddr, HEX);
  if (pluginStartAddr != 0x34) return false;

  return true;
}

void Adafruit_VS1053::stopRecordOgg(void) {
  sciWrite(VS1053_SCI_AICTRL3, 1);
}

void Adafruit_VS1053::startRecordOgg(boolean mic) {
  /* Set VS1053 mode bits as instructed in the VS1053b Ogg Vorbis Encoder
     manual. Note: for microphone input, leave SMF_LINE1 unset! */
  if (mic) {
    sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_ADPCM | VS1053_MODE_SM_SDINEW);
  } else {
    sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | 
	     VS1053_MODE_SM_ADPCM | VS1053_MODE_SM_SDINEW);
  }
  sciWrite(VS1053_SCI_AICTRL0, 1024);
  /* Rec level: 1024 = 1. If 0, use AGC */
  sciWrite(VS1053_SCI_AICTRL1, 1024);
  /* Maximum AGC level: 1024 = 1. Only used if SCI_AICTRL1 is set to 0. */
  sciWrite(VS1053_SCI_AICTRL2, 0);
  /* Miscellaneous bits that also must be set before recording. */
  sciWrite(VS1053_SCI_AICTRL3, 0);
  
  sciWrite(VS1053_SCI_AIADDR, 0x34);
  delay(1);    while (! readyForData() );
}

void Adafruit_VS1053::GPIO_pinMode(uint8_t i, uint8_t dir) {
  if (i > 7) return;

  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_DDR);
  uint16_t ddr = sciRead(VS1053_REG_WRAM);

  if (dir == INPUT)
    ddr &= ~_BV(i);
  if (dir == OUTPUT)
    ddr |= _BV(i);

  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_DDR);
  sciWrite(VS1053_REG_WRAM, ddr);
}


void Adafruit_VS1053::GPIO_digitalWrite(uint8_t val) {
  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_ODATA);
  sciWrite(VS1053_REG_WRAM, val);
}

void Adafruit_VS1053::GPIO_digitalWrite(uint8_t i, uint8_t val) {
  if (i > 7) return;

  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_ODATA);
  uint16_t pins = sciRead(VS1053_REG_WRAM);

  if (val == LOW)
    pins &= ~_BV(i);
  if (val == HIGH)
    pins |= _BV(i);

  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_ODATA);
  sciWrite(VS1053_REG_WRAM, pins);
}

uint16_t Adafruit_VS1053::GPIO_digitalRead(void) {
  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_IDATA);
  return sciRead(VS1053_REG_WRAM) & 0xFF;
}

boolean Adafruit_VS1053::GPIO_digitalRead(uint8_t i) {
  if (i > 7) return 0;

  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_IDATA);
  uint16_t val = sciRead(VS1053_REG_WRAM);
  if (val & _BV(i)) return true;
  return false;
}

uint16_t Adafruit_VS1053::sciRead(uint8_t addr) {
  uint16_t data;

  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.beginTransaction(VS1053_CONTROL_SPI_SETTING);
  #endif
  digitalWrite(_cs, LOW);  
  spiwrite(VS1053_SCI_READ);
  spiwrite(addr);
  delayMicroseconds(10);
  data = spiread();
  data <<= 8;
  data |= spiread();
  digitalWrite(_cs, HIGH);
  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.endTransaction();
  #endif

  return data;
}


void Adafruit_VS1053::sciWrite(uint8_t addr, uint16_t data) {
  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.beginTransaction(VS1053_CONTROL_SPI_SETTING);
  #endif
  digitalWrite(_cs, LOW);  
  spiwrite(VS1053_SCI_WRITE);
  spiwrite(addr);
  spiwrite(data >> 8);
  spiwrite(data & 0xFF);
  digitalWrite(_cs, HIGH);
  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.endTransaction();
  #endif
}



uint8_t Adafruit_VS1053::spiread(void)
{
  int8_t i, x;
  x = 0;

  // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low

  if (useHardwareSPI) {
    x = SPI.transfer(0x00);
  } else {
    for (i=7; i>=0; i--) {
      if ((*misoportreg) & misopin)
	x |= (1<<i);    
      *clkportreg |= clkpin;
      *clkportreg &= ~clkpin;
      //    asm("nop; nop");
    }
    // Make sure clock ends low
    *clkportreg &= ~clkpin;
  } 
  return x;
}

void Adafruit_VS1053::spiwrite(uint8_t c)
{

  uint8_t x __attribute__ ((aligned (32))) = c;
  spiwrite(&x, 1);
}


void Adafruit_VS1053::spiwrite(uint8_t *c, uint16_t num)
{
  // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low

  if (useHardwareSPI) {
    
    //#if defined(ESP32)  // optimized
    //  SPI.writeBytes(c, num);
    //  return;
    //#endif

    while (num--) {
      SPI.transfer(c[0]);
      c++;
    }

  } else {
    while (num--) {
      for (int8_t i=7; i>=0; i--) {
	*clkportreg &= ~clkpin;
	if (c[0] & (1<<i)) {
	  *mosiportreg |= mosipin;
	} else {
	  *mosiportreg &= ~mosipin;
	}
	*clkportreg |= clkpin;
      }
      *clkportreg &= ~clkpin;   // Make sure clock ends low

      c++;
    }
  }
}



void Adafruit_VS1053::sineTest(uint8_t n, uint16_t ms) {
  reset();
  
  uint16_t mode = sciRead(VS1053_REG_MODE);
  mode |= 0x0020;
  sciWrite(VS1053_REG_MODE, mode);

  while (!digitalRead(_dreq));
	 //  delay(10);

  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
  #endif
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
  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.endTransaction();
  #endif
  
  delay(ms);

  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
  #endif
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
  #ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI) SPI.endTransaction();
  #endif
}
