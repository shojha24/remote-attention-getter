#define SOFT_SPI_MISO_PIN // make sure to add pin numbers here
#define SOFT_SPI_MOSI_PIN //
#define SOFT_SPI_SCK_PIN //

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

#define CE_PIN 7
#define CSN_PIN 8

#define BUTTON_PIN 2
#define MOTOR_PIN 3

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

int button_state = 0;

// Let these addresses be used for the pair
uint8_t address[][6] = { "1Node", "2Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

// Used to control whether this node is sending or receiving
bool role = false;  // true = TX role, false = RX role

// Receiving a float from the other node that corresponds to the # device. Think of it as an ID.
float counter = 0.0;

// Here is the ID of the device
float id = 0.0;

void setup() {

  // Set motor and button pins
  pinMode(BUTTON_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);

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
  radio.openWritingPipe(address[0]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!0]);  // using pipe 1

  // additional setup specific to the node's role
  radio.startListening();  // put radio in RX mode

  // For debugging info
  // printf_begin();             // needed only once for printing details
  // radio.printDetails();       // (smaller) function that prints raw register values
  // radio.printPrettyDetails(); // (larger) function that prints human readable data

}  // setup

void loop() {

    button_state = digitalRead(BUTTON_PIN);

    uint8_t pipe;
    if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      radio.read(&counter, bytes);             // fetch payload from FIFO
      Serial.println(counter);                 // print the payload's value
      if (counter == id) {                     // check if the received counter corresponds to device id
        while (button_state == LOW) {          // executes code until button is pressed
          digitalWrite(MOTOR_PIN, HIGH);       // turn motor on
          delay(500);                          // wait 500 ms
          digitalWrite(MOTOR_PIN, LOW);        // turn motor off
          delay(500);                          // wait 500 ms
        }
        digitalWrite(MOTOR_PIN, LOW);          // turn motor off
      } else {
        digitalWrite(MOTOR_PIN, LOW);          // keep motor off if counter does not correspond to device id
      }
    }

}  // loop
