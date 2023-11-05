/*
  Using the LTC2473 16 Bit ADC
  By: Urs Utzinger
  Date: November 3, 2023
  License: This code is public domain but you can buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to read ADC values from two devices
*/

#include <Wire.h>
#include <LTC2473.h>

TwoWire myWire = TwoWire(0);              // Initialize the two wire system.
LTC2473 adc_1(LTC2473_I2C_ADDRESS_GND);     // Initialize the sensor construct
LTC2473 adc_2(LTC2473_I2C_ADDRESS_VCC);     // Initialize the sensor construct
float voltage = 0.0;
uint8_t error = 0;

void setup() {

  delay(3000);                            // Assure catching setup log, unfortunately "while(!Serial);" is not working
  Serial.begin(115200);
  Serial.println("LTC2473 Demo");

  myWire.begin();                         // Start I2C interface

  adc_1.begin(myWire);
  if ( adc_1.available ) { Serial.println("Sensor 1: set up."); } 
  else                   { Serial.println("Sensor 1: not present."); }

  adc_2.begin(myWire);
  if ( adc_2.available ) { Serial.println("Sensor 2: set up."); } 
  else                   { Serial.println("Sensor 2: not present."); }

} // end setup
    
void loop() {
  if (adc_1.available) {
      error = adc_1.read(&voltage); 
      if (error == 0) { Serial.printf("Voltage 1: %.5f\n", voltage); }
      else { Serial.printf("Could not read sensor 1. Error %d\n", error); } 
  }
  if (adc_2.available) {
      error = adc_2.read(&voltage); 
      if (error == 0) { Serial.printf("Voltage 2: %.5f\n", voltage); }
      else { Serial.printf("Could not read sensor 2. Error %d\n", error); } 
  }
  delay(100);
}
