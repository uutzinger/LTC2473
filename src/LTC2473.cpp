/*
  This is a library written for the LTC2473 ADC->I2C.

  Written by Urs Utzinger, 2023

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
  rate. IT useS a single 2.7V to 5.5V supply and communicates through an I2C
  Interface 100 or 400 kbit/sec. The LTC2473 is differential with a +-VREF input range. 
  The ADC includes a 1.25V integrated reference with 2ppm/C drift performance and 
  0.1% initial accuracy. It includes an integrated oscillator and perform 
  conversions with no latency for multiplexed applications. 
  The LTC2473 includes a proprietary input sampling scheme that reduces 
  the average input current several orders of magnitude when compared to 
  conventional delta sigma converters.

  http://www.linear.com/product/LTC2473
  http://www.linear.com/product/LTC2473#demoboards
*/

#include "LTC2473.h"

LTC2473::LTC2473(uint8_t i2cAddress) : addr(i2cAddress) {
    // Constructor implementation (if needed)
}

#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
// Teensy 3.6
uint8_t LTC2473::begin(i2c_t3 &i2cPort)
#else
void LTC2473::begin(TwoWire &i2cPort)
#endif
{
  wire = &i2cPort;
  byte error = ack(); // check if device is present
  available = (error == 0); // keep track of device availability
  delay(10);
}

// Returns false if I2C device ack's
int8_t LTC2473::ack()
{
  wire->beginTransmission((uint8_t)addr);
  return((int8_t)wire->endTransmission(true)); // end connection
}

// Reads 16bit data from LTC2473 without specifying register
int8_t LTC2473::read(float *voltage, bool stop)
{  
  int8_t i;
  uint8_t ret = 0;

  twoBytes data;
  i = 2;
  ret = wire->requestFrom((uint8_t)addr, (uint8_t)2, (uint8_t)stop); // request 2 bytes from the device
  while (wire->available())  {
    i--;
    if (i >= 0) { data.byte[i] = wire->read(); }
  }
  if (i == 0) { *voltage = code_to_voltage(data.uint16); } 
  return(i);

}

// Set 833 sps mode
int8_t LTC2473::highspeed() {
  return(write(LTC2473_SPS_833));
}

// Set 208 sps mode
int8_t LTC2473::regularspeed() {
  return(write(LTC2473_SPS_208));
}

// Set vref to go to sleep, after read takes 12ms to wake
int8_t LTC2473::sleep() {
  return(write(LTC2473_SLEEP));
}

// Program LTC2473 with 8-bit command
int8_t LTC2473::write(uint8_t command)
{  
  wire->beginTransmission((uint8_t)addr);
  wire->write((uint8_t)command);
  return((int8_t)wire->endTransmission(true)); // end connection
}

// Calculates the voltage corresponding to an adc code, given the reference (in volts)
float LTC2473::code_to_voltage(uint16_t adc_code)
{
  // >= Vref  : 65535
  //      0V  : 32768
  // <= -Vref :    0
  float voltage = (float) adc_code;
  voltage = voltage - 32768.0;         // 1) Subtract offset
  voltage = voltage / 32767.0;         // 2) Fraction of max
  voltage = voltage * LTC2471_VREF;    // 2) Multiply fraction by Vref to get the actual voltage at the input (in volts)
  return(voltage);
}
