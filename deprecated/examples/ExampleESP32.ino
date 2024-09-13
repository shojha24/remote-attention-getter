#include <SPI.h>
#include <RF24.h>
 
// to use custom-defined pins, uncomment the following
// #define MY_MISO 26
// #define MY_MOSI 27
// #define MY_SCLK 25
// #define MY_SS   32  // pass MY_SS as the csn_pin parameter to the RF24 constructor
 
// notice these pin numbers are not the same used in the library examples
RF24 radio(2, 0); // the (ce_pin, csn_pin) connected to the radio
 
SPIClass* hspi = nullptr; // we'll instantiate this in the `setup()` function
// by default the HSPI bus predefines the following pins
// HSPI_MISO = 12
// HSPI_MOSI = 13
// HSPI_SCLK = 14
// HSPI_SS   = 15
 
void setup() {
  Serial.begin(115200);
  while (!Serial) {} //some boards need this
 
  hspi = new SPIClass(HSPI); // by default VSPI is used
  hspi->begin();
  // to use the custom defined pins, uncomment the following
  // hspi->begin(MY_SCLK, MY_MISO, MY_MOSI, MY_SS)
 
  if (!radio.begin(hspi)) {
    Serial.println(F("radio hardware not responding!!"));
    while (1) {} // hold program in infinite loop to prevent subsequent errors
  }
 
  // ... continue with program as normal (see library examples/ folder)
}