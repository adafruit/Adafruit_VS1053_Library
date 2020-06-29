/*!
 * @file Adafruit_VS1053.h
 */

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
#elif defined(__arm__)
#if defined(TEENSYDUINO)
typedef volatile uint8_t RwReg;
typedef uint8_t PortMask;
#else
typedef volatile uint32_t RwReg;
typedef uint32_t PortMask;
#endif
#elif defined(ESP8266) || defined(ESP32)
typedef volatile uint32_t RwReg;
typedef uint32_t PortMask;
#elif defined(__ARDUINO_ARC__)
typedef volatile uint32_t RwReg;
typedef uint32_t PortMask;
#else
typedef volatile uint8_t RwReg; //!< 1-byte read-write register
typedef uint8_t PortMask; //!< Type definition for a bitmask that is used to
                          //!< specify the bit width
#endif

typedef volatile RwReg PortReg; //!< Type definition/alias used to specify the
                                //!< port register that a pin is in

#define VS1053_FILEPLAYER_TIMER0_INT                                           \
  255 //!< Allows useInterrupt to accept pins 0 to 254
#define VS1053_FILEPLAYER_PIN_INT                                              \
  5 //!< Allows useInterrupt to accept pins 0 to 4

#define VS1053_SCI_READ 0x03  //!< Serial read address
#define VS1053_SCI_WRITE 0x02 //!< Serial write address

#define VS1053_REG_MODE 0x00       //!< Mode control
#define VS1053_REG_STATUS 0x01     //!< Status of VS1053b
#define VS1053_REG_BASS 0x02       //!< Built-in bass/treble control
#define VS1053_REG_CLOCKF 0x03     //!< Clock frequency + multiplier
#define VS1053_REG_DECODETIME 0x04 //!< Decode time in seconds
#define VS1053_REG_AUDATA 0x05     //!< Misc. audio data
#define VS1053_REG_WRAM 0x06       //!< RAM write/read
#define VS1053_REG_WRAMADDR 0x07   //!< Base address for RAM write/read
#define VS1053_REG_HDAT0 0x08      //!< Stream header data 0
#define VS1053_REG_HDAT1 0x09      //!< Stream header data 1
#define VS1053_REG_VOLUME 0x0B     //!< Volume control

#define VS1053_GPIO_DDR 0xC017   //!< Direction
#define VS1053_GPIO_IDATA 0xC018 //!< Values read from pins
#define VS1053_GPIO_ODATA 0xC019 //!< Values set to the pins

#define VS1053_INT_ENABLE 0xC01A //!< Interrupt enable

#define VS1053_MODE_SM_DIFF                                                    \
  0x0001 //!< Differential, 0: normal in-phase audio, 1: left channel inverted
#define VS1053_MODE_SM_LAYER12 0x0002  //!< Allow MPEG layers I & II
#define VS1053_MODE_SM_RESET 0x0004    //!< Soft reset
#define VS1053_MODE_SM_CANCEL 0x0008   //!< Cancel decoding current file
#define VS1053_MODE_SM_EARSPKLO 0x0010 //!< EarSpeaker low setting
#define VS1053_MODE_SM_TESTS 0x0020    //!< Allow SDI tests
#define VS1053_MODE_SM_STREAM 0x0040   //!< Stream mode
#define VS1053_MODE_SM_SDINEW 0x0800   //!< VS1002 native SPI modes
#define VS1053_MODE_SM_ADPCM 0x1000    //!< PCM/ADPCM recording active
#define VS1053_MODE_SM_LINE1 0x4000 //!< MIC/LINE1 selector, 0: MICP, 1: LINE1
#define VS1053_MODE_SM_CLKRANGE                                                \
  0x8000 //!< Input clock range, 0: 12..13 MHz, 1: 24..26 MHz

#define VS1053_SCI_AIADDR                                                      \
  0x0A //!< Indicates the start address of the application code written earlier
       //!< with SCI_WRAMADDR and SCI_WRAM registers.
#define VS1053_SCI_AICTRL0                                                     \
  0x0C //!< SCI_AICTRL register 0. Used to access the user's application program
#define VS1053_SCI_AICTRL1                                                     \
  0x0D //!< SCI_AICTRL register 1. Used to access the user's application program
#define VS1053_SCI_AICTRL2                                                     \
  0x0E //!< SCI_AICTRL register 2. Used to access the user's application program
#define VS1053_SCI_AICTRL3                                                     \
  0x0F //!< SCI_AICTRL register 3. Used to access the user's application program

#define VS1053_DATABUFFERLEN 32 //!< Length of the data buffer

/*!
 * Driver for the Adafruit VS1053
 */
class Adafruit_VS1053 {
public:
  /*!
   * @brief Software SPI constructor - must specify all pins
   * @param mosi MOSI (Microcontroller Out Serial In) pin
   * @param miso MISO (Microcontroller In Serial Out) pin
   * @param clk Clock pin
   * @param rst Reset pin
   * @param cs SCI Chip Select pin
   * @param dcs SDI Chip Select pin
   * @param dreq Data Request pin
   */
  Adafruit_VS1053(int8_t mosi, int8_t miso, int8_t clk, int8_t rst, int8_t cs,
                  int8_t dcs, int8_t dreq);
  /*!
   * @brief Hardware SPI constructor - assumes hardware SPI pins
   * @param rst Reset pin
   * @param cs SCI Chip Select pin
   * @param dcs SDI Chip Select pin
   * @param dreq Data Request pin
   */
  Adafruit_VS1053(int8_t rst, int8_t cs, int8_t dcs, int8_t dreq);
  /*!
   * @brief Initialize communication and (hard) reset the chip.
   * @return Returns true if a VS1053 is found
   */
  uint8_t begin(void);
  /*!
   * @brief Performs a hard reset of the chip
   */
  void reset(void);
  /*!
   * @brief Attempts a soft reset of the chip
   */
  void softReset(void);
  /*!
   * @brief Reads from the specified register on the chip
   * @param addr Register address to read from
   * @return Retuns the 16-bit data corresponding to the received address
   */
  uint16_t sciRead(uint8_t addr);
  /*!
   * @brief Writes to the specified register on the chip
   * @param addr Register address to write to
   * @param data Data to write
   */
  void sciWrite(uint8_t addr, uint16_t data);
  /*!
   * @brief Generate a sine-wave test signal
   * @param n Defines the sine test to use
   * @param ms Delay (in ms)
   */
  void sineTest(uint8_t n, uint16_t ms);
  /*!
   * @brief Low-level SPI write operation
   * @param d What to write
   */
  void spiwrite(uint8_t d);
  /*!
   * @brief Low-level SPI write operation
   * @param c Pointer to a buffer containing the data to send
   * @param num How many elements in the buffer should be sent
   */
  void spiwrite(uint8_t *c, uint16_t num);
  /*!
   * @brief Low-level SPI read operation
   * @return Returns a byte read from SPI
   */
  uint8_t spiread(void);

  /*!
   * @brief Reads the DECODETIME register from the chip
   * @return Returns the decode time as an unsigned 16-bit integer
   */
  uint16_t decodeTime(void);
  /*!
   * @brief Set the output volume for the chip
   * @param left Desired left channel volume
   * @param right Desired right channel volume
   */
  void setVolume(uint8_t left, uint8_t right);
  /*!
   * @brief Prints the contents of the MODE, STATUS, CLOCKF and VOLUME registers
   */
  void dumpRegs(void);

  /*!
   * @brief Decode and play the contents of the supplied buffer
   * @param buffer Buffer to decode and play
   * @param buffsiz Size to decode and play
   */
  void playData(uint8_t *buffer, uint8_t buffsiz);
  /*!
   * @brief Test if ready for more data
   * @return Returns true if it is ready for data
   */
  boolean readyForData(void);
  /*!
   * @brief Apply a code patch
   * @param patch Patch to apply
   * @param patchsize Patch size
   */
  void applyPatch(const uint16_t *patch, uint16_t patchsize);
  /*!
   * @brief Load the specified plug-in
   * @param fn Plug-in to load
   * @return Either returns 0xFFFF if there is an error, or the address of the
   * plugin that was loaded
   */
  uint16_t loadPlugin(char *fn);

  /*!
   * @brief Write to a GPIO pin
   * @param i GPIO pin to write to
   * @param val Value to write
   */
  void GPIO_digitalWrite(uint8_t i, uint8_t val);
  /*!
   * @brief Write to all 8 GPIO pins at once
   * @param i Value to write
   */
  void GPIO_digitalWrite(uint8_t i);
  /*!
   * @brief Read all 8 GPIO pins at once
   * @return Returns a 2 byte value with the reads from the 8 pins
   */
  uint16_t GPIO_digitalRead(void);
  /*!
   * @brief Read a single GPIO pin
   * @param i pin to read
   * @return Returns the state of the specified GPIO pin
   */
  boolean GPIO_digitalRead(uint8_t i);
  /*!
   * @brief Set the Pin Mode (INPUT/OUTPUT) for a GPIO pin.
   * @param i Pin to set the mode for
   * @param dir Mode to set
   */
  void GPIO_pinMode(uint8_t i, uint8_t dir);

  /*!
   * @brief Initialize chip for OGG recording
   * @param plugin Binary file of the plugin to use
   * @return Returns true if the device is ready to record
   */
  boolean prepareRecordOgg(char *plugin);
  /*!
   * @brief Start recording
   * @param mic mic=true for microphone input
   */
  void startRecordOgg(boolean mic);
  /*!
   * @brief Stop the recording
   */
  void stopRecordOgg(void);
  /*!
   * @brief Returns the number of words recorded
   * @return 2-byte unsigned int with the number of words
   */
  uint16_t recordedWordsWaiting(void);
  /*!
   * @brief Reads the next word from the buffer of recorded words
   * @return Returns the 16-bit data corresponding to the received address
   */
  uint16_t recordedReadWord(void);

  uint8_t mp3buffer[VS1053_DATABUFFERLEN]; //!< mp3 buffer that gets sent to the
                                           //!< device

#ifdef ARDUINO_ARCH_SAMD
protected:
  uint32_t _dreq;

private:
  int32_t _mosi, _miso, _clk, _reset, _cs, _dcs;
  boolean useHardwareSPI;
#else
protected:
  uint8_t _dreq; //!< Data request pin
private:
  int8_t _mosi, _miso, _clk, _reset, _cs, _dcs;
  boolean useHardwareSPI;
#endif
};

/*!
 * @brief File player for the Adafruit VS1053
 */
class Adafruit_VS1053_FilePlayer : public Adafruit_VS1053 {
public:
  /*!
   * @brief Software SPI constructor. Uses Software SPI, so you must specify all
   * SPI pins
   * @param mosi MOSI (Microcontroller Out Serial In) pin
   * @param miso MISO (Microcontroller In Serial Out) pin
   * @param clk Clock pin
   * @param rst Reset pin
   * @param cs SCI Chip Select pin
   * @param dcs SDI Chip Select pin
   * @param dreq Data Request pin
   * @param cardCS CS pin for the SD card on the SPI bus
   */
  Adafruit_VS1053_FilePlayer(int8_t mosi, int8_t miso, int8_t clk, int8_t rst,
                             int8_t cs, int8_t dcs, int8_t dreq, int8_t cardCS);
  /*!
   * @brief Hardware SPI constructor. Uses Hardware SPI and assumes the default
   * SPI pins
   * @param rst Reset pin
   * @param cs SCI Chip Select pin
   * @param dcs SDI Chip Select pin
   * @param dreq Data Request pin
   * @param cardCS CS pin for the SD card on the SPI bus
   */
  Adafruit_VS1053_FilePlayer(int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
                             int8_t cardCS);

  /*!
   * @brief Hardware SPI constructor. Uses Hardware SPI and assumes the default
   * SPI pins
   * @param cs SCI Chip Select pin
   * @param dcs SDI Chip Select pin
   * @param dreq Data Request pin
   * @param cardCS CS pin for the SD card on the SPI bus
   */
  Adafruit_VS1053_FilePlayer(int8_t cs, int8_t dcs, int8_t dreq, int8_t cardCS);

  /*!
   * @brief Initialize communication and reset the chip.
   * @return Returns true if a VS1053 is found
   */
  boolean begin(void);
  /*!
   * @brief Specifies the argument to use for interrupt-driven playback
   * @param type interrupt to use. Valid arguments are
   * VS1053_FILEPLAYER_TIMER0_INT and VS1053_FILEPLAYER_PIN_INT
   * @return Returs true/false for success/failure
   */
  boolean useInterrupt(uint8_t type);
  File currentTrack;             //!< File that is currently playing
  volatile boolean playingMusic; //!< Whether or not music is playing
  /*!
   * @brief Feeds the buffer. Reads mp3 file data from the SD card and file and
   * puts it into the buffer that the decoder reads from to play a file
   */
  void feedBuffer(void);
  /*!
   * @brief Checks if the inputted filename is an mp3
   * @param fileName File to check
   * @return Returns true or false
   */
  static boolean isMP3File(const char *fileName);
  /*!
   * @brief Checks for an ID3 tag at the beginning of the file.
   * @param mp3 File to read
   * @return returns the seek position within the file where the mp3 data starts
   */
  unsigned long mp3_ID3Jumper(File mp3);
  /*!
   * @brief Begin playing the specified file from the SD card using
   * interrupt-drive playback.
   * @param *trackname File to play
   * @return Returns true when file starts playing
   */
  boolean startPlayingFile(const char *trackname);
  /*!
   * @brief Play the complete file. This function will not return until the
   * playback is complete
   * @param *trackname File to play
   * @return Returns true when file starts playing
   */
  boolean playFullFile(const char *trackname);
  void stopPlaying(void); //!< Stop playback
  /*!
   * @brief If playback is paused
   * @return Returns true if playback is paused
   */
  boolean paused(void);
  /*!
   * @brief If playback is stopped
   * @return Returns true if playback is stopped
   */
  boolean stopped(void);
  /*!
   * @brief Pause playback
   * @param pause whether or not to pause playback
   */
  void pausePlaying(boolean pause);

private:
  void feedBuffer_noLock(void);

  uint8_t _cardCS;
};

#endif // ADAFRUIT_VS1053_H
