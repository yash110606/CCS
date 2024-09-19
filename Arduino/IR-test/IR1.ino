#include <IRremote.h>

// Set the IR LED pin
const int irLedPin = 3;

// Create the IR sender object
IRsend irsend(irLedPin);

void setup() {
  // Initialize the IR sender
  IrSender.begin();
}

void loop() {
  // HEX code to be sent using NEC protocol (example: 0xF20D)
  unsigned long hexCode = 0xF20D;

  // Transmit the HEX code using NEC protocol
  irsend.sendNEC(hexCode, 32); // 32 is the number of bits in NEC protocol
  
  // Wait before sending again
  delay(2000); // Send the signal every 2 seconds
}
