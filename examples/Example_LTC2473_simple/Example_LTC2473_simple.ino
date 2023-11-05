/*
  Using the LTC2473 16 Bit ADC
  By: Urs Utzinger
  Date: November 3, 2023
  License: This code is public domain but you can buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to read ADC value.
*/

#include <Wire.h>
#include <LTC2473.h>

TwoWire myWire = TwoWire(0);              // Initialize the two wire system.
LTC2473 adc(LTC2473_I2C_ADDRESS_GND);     // Initialize the sensor construct
float voltage = 0.0;
uint8_t error = 0;

void setup() {

  delay(3000);                            // Assure catching setup log, unfortunately "while(!Serial);" is not working
  Serial.begin(115200);
  Serial.println("LTC2473 Demo");

  myWire.begin();                         // Start I2C interface

  adc.begin(myWire);
  if ( adc.available ) { Serial.println("Sensor: set up."); } 
  else                 { Serial.println("Sensor: not present."); }

} // end setup
    
void loop() {
  if (adc.available) {
      error = adc.read(&voltage); 
      if (error == 0) { Serial.printf("Voltage: %.4f\n", voltage); }
      else { Serial.printf("Could not read sensor. Error %d\n", error); } 
  }
  delay(100);
}
