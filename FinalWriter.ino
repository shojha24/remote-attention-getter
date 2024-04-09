#define SOFT_SPI_MISO_PIN 
#define SOFT_SPI_MOSI_PIN
#define SOFT_SPI_SCK_PIN

#include <DigitalIO.h>
#include <Encoder.h>
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>
#include <SPI.h>

/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
*/

// Define rotary encoder pins
#define ENC_A 2
#define ENC_B 3
#define PUSH 4

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile float counter = 0.0;

#define CE_PIN 7
#define CSN_PIN 8
// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
uint8_t address[][6] = { "1Node", "2Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit


void setup() {

  // Set encoder pins and attach interrupts
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(PUSH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(counter));  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[1]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!1]);  // using pipe 1

  // additional setup specific to the node's role
  radio.stopListening();  // put radio in TX mode

  // For debugging info
  // printf_begin();             // needed only once for printing details
  // radio.printDetails();       // (smaller) function that prints raw register values
  // radio.printPrettyDetails(); // (larger) function that prints human readable data

}  // setup

void loop() {

    static int lastCounter = 0;

    // If count has changed print the new value to serial
    if(counter != lastCounter){
      Serial.println(counter);
      lastCounter = counter;
    }

    bool currentState = digitalRead(PUSH);
    if (currentState == false) {
      
      unsigned long start_timer = micros();                // start the timer
      bool report = radio.write(&counter, sizeof(float));  // transmit & save the report
      unsigned long end_timer = micros();                  // end the timer

      if (report) {
        Serial.print(F("Transmission successful! "));  // payload was delivered
        Serial.print(F("Time to transmit = "));
        Serial.print(end_timer - start_timer);  // print the timer result
        Serial.print(F(" us. Sent: "));
        Serial.println(counter);  // print the counter sent
      } else {
        Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
      }

      while (digitalRead(PUSH) == false) {
      // do nothing
      }
    }

}  // loop

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }

  // Keeps counter between 0 and 19. Counter cycles from 19 back to 0 upon full rotation. One-to-one correspondence between the ids and the encoder values is established.
  counter = (counter % 20) + 20 % 20;             


  // a % b is negative, but we want counter to always be between 0 and 19
  // ((a % b) + b) % b

} 