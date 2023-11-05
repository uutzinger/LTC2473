/*
  This is a library written for the LTC2473 ADC->I2C.

  Written by Urs Utzinger 2023

  The sensor uses I2C to communicate.

  https://github.com/uutzinger/LTC2473

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  The LTC2473 is a small, 16-bit analog-to-digital converters with an
  integrated precision reference and a selectable 208sps or 833sps output
  rate. IT useS a single 2.7V to 5.5V supply and communicateS through an I2C
  Interface 100 or 400 kbit/s. the LTC2473 is differential with a +-VREF input range. 
  The ADC's include a 1.25V integrated reference with 2ppm/C drift performance and 
  0.1% initial accuracy. It includes an integrated oscillator and perform 
  conversions with no latency for multiplexed applications. 
  The LTC2473 includes a proprietary input sampling scheme that reduces 
  the average input current several orders of magnitude when compared to 
  conventional delta sigma converters.

  http://www.linear.com/product/LTC2473
  http://www.linear.com/product/LTC2473#demoboards

  I2C DATA FORMAT (MSB FIRST);

  Data Out: (read value)
  =========
  Byte #
  START  SA6 SA5 SA4 SA3 SA2 SA1 SA0 W SACK   REPEAT START

  Byte #2  MSB
  D15 D14 D13 D12 D11 D10 D9 D8 MACK

  Byte #3  LSB
  D7 D6 D5 D4 D3 D2 D1 D0 MNACK STOP

  Data In: (send 4bit command, can send 8 bit command and 4 bits will be ignored)
  ========
  Byte #1                                     Byte #2
  START  SA6 SA5 SA4 SA3 SA2 SA1 SA0 W SACK   C3 C2 C1 C0 X X X X SACK STOP

  START        : I2C Start
  REPEAT Start : I2C Repeat Start
  STOP         : I2C Stop
  SACK         : I2C Slave Generated Acknowledge (Active Low)
  MACK         : I2C Master Generated Acknowledge (Active Low)
  MNACK        : I2C Master Generated Not Acknowledge
  SAx  : I2C Address
  W    : I2C Write (0)
  R    : I2C Read  (1)
  Cx   : Command Code
  Dx   : Data Bits
  X    : Don't care

*/

#pragma once

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif // (ARDUINO >= 100)

#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
// Teensy
#include "i2c_t3.h"

#else

#include "Wire.h"

// The catch-all default is 32
#ifndef I2C_BUFFER_LENGTH
#define I2C_BUFFER_LENGTH 32
#endif // I2C_BUFFER_LENGTH

#endif // defined(__MK64FX512__) || defined(__MK66FX1M0__)

// I2C Address
#define LTC2473_I2C_ADDRESS_GND (0x14) // address pin pulled low
#define LTC2473_I2C_ADDRESS_VCC (0x54) // address pin pulled high

// Mode Configuration
// Register bits: C7, C6, C5, C4, C3, C2, C1, C0 
// C7 EN1: high: enable program mode low: not specified
// C6 EN2: high: not specified,      low: enable program mode
// C5 SPD: high: 833 sps,            low: 208 sps (default) 
// C4 SLP: high: sleep,              low: Nap mode (default)
// C3..C0: don't care
// Sleep mode: reference and converters are turned off after reading, it takes 12ms to power up, sensor takes 0.0002mA
// Nap mode: reference and converters are kept on, sensor takes 0.8mA
#define LTC2473_SPS_208 (0b10000000) // 0x80
#define LTC2473_SPS_833 (0b10100000) // 0xA0
#define LTC2473_SLEEP   (0b10010000) // 0x90

// Reference Voltage
#define LTC2471_VREF (1.250) // 1.25V internal voltage reference

union fourBytes
{
  int32_t int32;       //!< 32-bit signed integer to be converted to four bytes
  uint32_t uint32;     //!< 32-bit unsigned integer to be converted to four bytes
  uint8_t byte[4];     //!< 4 bytes (unsigned 8-bit integers) to be converted to a 32-bit signed or unsigned integer
};

union twoBytes
{
  int16_t int16;       //!< 16-bit signed integer to be converted to two bytes
  uint16_t uint16;     //!< 16-bit unsigned integer to be converted to two bytes
  uint8_t byte[2];     //!< 2 bytes (unsigned 8-bit integers) to be converted to a 16-bit signed or unsigned integer
};

class LTC2473
{
public:
  explicit LTC2473(uint8_t i2cAddress = LTC2473_I2C_ADDRESS_GND);
  // variables
// By default use Wire, standard I2C speed, and the default ADS1015 address
#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
	// Teensy
	void    begin(i2c_t3 &wirePort);
#else
	void    begin(TwoWire &wirePort);
#endif // defined(__MK64FX512__) || defined(__MK66FX1M0__)
  bool    available;                              // Indicates if sensor is attached to the I2C bus 

  // functions
	int8_t  ack();                                  // Checks if sensor is busy
	int8_t  sleep();                                // Put the sensor to sleep
	int8_t  highspeed();                            // Set the sensor to high speed mode
	int8_t  regularspeed();                         // Set the sensor to low speed mode
	int8_t  read(float *voltage, bool stop=true);   // Read 16bit data from LTC2473

private:
  // variables
#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
	// Teensy
	i2c_t3 *wire;
#else
	TwoWire *wire;
#endif // defined(__MK64FX512__) || defined(__MK66FX1M0__)
	uint8_t addr;
  // functions
	float code_to_voltage(uint16_t adc_code);       // Convert 16bit data to voltage
	int8_t write(uint8_t command);	                  // Write 8bit command to LTC2473
};
