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
float voltage_1 = 0.0;
float voltage_2 = 0.0;
float temperature_1;
float temperature_2;
int8_t error=0;
uint32_t current_time = micros();
uint32_t last_time = micros();
uint16_t rate = 0;
float cpu_usage = 0.0;

float voltage_to_temperature(float V) {
  /*
  Convert A/D reading from a full bridge to temperature in C.
  This assumes thermistor is connected to GND and there is a load resistor to Vref.
  This assumes there is voltage divider built with same load resistors connetedd to GND and Vref.
  IN- is connected to thermistor.
  IN+ is connetected to voltage divider.
  Standard Stein Hartman equation is used model the thermistor.
  Change values below for your thermistor
  R_load the resistors in your full bridge
  R_nominal the thermistor value at T=25C
  R_beta the thermistor characteristics
  V_ref the voltage going into the full bridge
  T_25 25C in Kelvin
  */

  /*
  Math to solve for temperature
  =============================

  v0 = V_ref / 2.0;
  v1 = V_ref * (R/(R+R_load));
  V = v0-v1;
  V = V_ref * (1/2.0 - R/(R+R_load))
  
  Solve for R

  V/V_ref = 0.5 - R/(R+R_load)
  V/V_ref - 0.5 = - R/(R+R_load)
  V/V_ref*(R+R_load) - 0.5*(R+R_load) = -R
  V/V_ref*R + V/V_ref*R_load - 0.5*R -0.5*R_load) = -R
  V/V_ref*R + 0.5*R -0.5*R_load + V/V_ref*R_load = 0
  R * (V/V_ref + 0.5) = 0.5*R_load - V/V_ref*R_load
  R * (V/V_ref + 0.5) = R_load * (0.5 - V/V_ref)
  R  = R_load * (0.5 - V/V_ref) / (V/V_ref + 0.5)
  
  Stein Hartman Equation
  R=R_nominal * exp({R_beta * (1/T - 1/T_25))

  Solve for T
  R_inf = R_nominal * exp(-R_beta/T_25)
  T = R_beta / log(R/R_inf) - 273.15;

  */

#define R_load    10000 
#define R_nominal 10000 
#define R_beta    3380
#define V_ref     1.250
#define T_25      298.15
#define R_inf     0.11928553845185222

  float R  = R_load * (0.5 - V/V_ref) / (0.5 + V/V_ref);
  float T = R_beta / log(R/R_inf) - 273.15;
  return(T);
}

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
      error = adc_1.read(&voltage_1, false); // do not send stop
      if (error == 0) { 
        temperature_1 = voltage_to_temperature(voltage_1);
        Serial.printf("T1: %+0.5f C, SPS: %d, CPU: %0.0f %%\n", temperature_1, rate, cpu_usage);
      } else {
        Serial.printf("Could not read sensor 1. Error %d\n", error); 
      } 
  }

  if (adc_2.available) {
      error = adc_2.read(&voltage_2, false); // do not send stop
      if (error == 0) { 
        temperature_2 = voltage_to_temperature(voltage_2);
        Serial.printf("T2: %+0.5f C, SPS: %d, CPU: %0.0f %%\n", temperature_2, rate, cpu_usage);
      } else {
        Serial.printf("Could not read sensor 2. Error %d\n", error); 
      } 
  }

  // Delay to achieve max sps, we want to run main loop at 833 Hz
  int32_t elapsedTime = (int32_t) (micros() - current_time); // will be positive number
  int32_t delayTime = MICROS_PER_SAMPLE - elapsedTime; // might become negative, if program can not keep up
  if (delayTime > 0) {
    cpu_usage = 100.0* (float)delayTime / MICROS_PER_SAMPLE;
    delayMicroseconds(static_cast<uint32_t>(delayTime)); // we can run at up to 833 samples per second at high speed and 208 samples per second at regular speed
  } 
}
