/*
  Using the LTC2473 16 Bit ADC
  By: Urs Utzinger
  Date: November 3, 2023
  License: This code is public domain but you can buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to output ADC values.
*/

#include <Wire.h>
#include <LTC2473.h>

#define I2C_FAST           400000           // Fast   400,000 but can also downgrade to regular
#define I2C_REGULAR        100000           // Normal 100,000
#define I2C_SLOW            50000           // Slow    50,000

// i2c clock stretch limit
#define I2C_DEFAULT_TIMEOUT     1           // these values are in milli seconds, normal timeout is 230 micro seconds
#define I2C_LONNG_TIMEOUT     200           // this values in in milli seconds, slow timtout is 200 milli seconds
#define I2C_BUFFERSIZE         32           // 32 is default an minimal size for ESP32

// 1,000,000 [us/ s] / 833 [sps]
#define MICROS_PER_SAMPLE    1196

TwoWire myWire = TwoWire(0);                // Initialize the two wire system.
LTC2473 adc_1(LTC2473_I2C_ADDRESS_GND);     // Initialize the sensor construct
LTC2473 adc_2(LTC2473_I2C_ADDRESS_VCC);     // Initialize the sensor construct
float voltage = 0.0;
int8_t error=0;
uint32_t current_time = micros();
uint32_t last_time = micros();
uint16_t rate = 0;

void setup() {

  delay(3000);                                 // Assure catching setup log, unfortunately "while(!Serial);" is not working
  Serial.begin(115200);
  Serial.println("LTC2473 Demo");

  myWire.begin(3,4);                           // Start I2C interface
  myWire.setClock((uint32_t) I2C_FAST);        // 400kHz clock
  myWire.setBufferSize(I2C_BUFFERSIZE);        // 
  myWire.setTimeout((uint16_t) I2C_DEFAULT_TIMEOUT);

  adc_1.begin(myWire);
  if ( adc_1.available ) { 
    Serial.println("Sensor 1: set up."); 
    error = adc_1.highspeed();
    if ( error == 0 ) { Serial.println("Sensor 1: highspeed set."); } 
    else              { Serial.printf("Sensor 1: failed to set highspeed. Error %d\n", error); }
  } else { Serial.println("Sensor 1: not present."); }

  adc_2.begin(myWire);
  if ( adc_2.available ) { 
    Serial.println("Sensor 2: set up."); 
    error = adc_2.highspeed();
    if ( error == 0 ) { Serial.println("Sensor 2: highspeed set."); } 
    else              { Serial.printf("Sensor 2: failed to set highspeed. Error %d\n", error); }
  } else { Serial.println("Sensor 2: not present."); }

} // end setup
    
void loop() {
  current_time = micros();

  // Calculate sampling rate
  uint16_t delta_time = current_time - last_time;
  if (delta_time > 0) { rate = 1000000 / delta_time; }
  last_time = current_time;

  if (adc_1.available) {
      error = adc_1.read(&voltage, false); // do not send stop
      if (error == 0) { 
        Serial.printf("V1: %+0.5f, SPS: %d\n", voltage, rate);
      } else {
        Serial.printf("Could not read sensor 1. Error %d\n", error); 
      } 
  }

  if (adc_2.available) {
      error = adc_2.read(&voltage, false); // do not send stop
      if (error == 0) { 
        Serial.printf("V2: %+0.5f, SPS: %d\n", voltage, rate);
      } else {
        Serial.printf("Could not read sensor 2. Error %d\n", error); 
      } 
  }

  // Delay to achieve max sps, we want to run main loop at 833 Hz
  int32_t elapsedTime = (int32_t) (micros() - current_time); // will be positive number
  int32_t delayTime = MICROS_PER_SAMPLE - elapsedTime; // might become negative, if program can not keep up
  if (delayTime > 0) {
    delayMicroseconds(static_cast<uint32_t>(delayTime)); // we can run at up to 833 samples per second at high speed and 208 samples per second at regular speed
  } 
}
