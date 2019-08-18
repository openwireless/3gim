// 4GIM(V1) sample sketch for Uno/Pro.. -- use $ commands
//
// @note
//  4GIM(RX)	-> Arduino Uno(D5)
//	4GIM(TX)	-> Arduino Uno(D4)
//	4GIM(IOREF)	-> Arduino Uno(5V)
//	4GIM(VCC)	-> Li-ion battery etc..(3.3-4.2V, DO NOT CONNECT 5V!!)
//	4GIM(GND)	-> Arduino Uno(GND)
//

#include <SoftwareSerial.h>

#define  ioSerial     Serial
SoftwareSerial gimSerial(4, 5);

const unsigned long baudrate = 115200;

void setup() {
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  delay(3000);
  digitalWrite(7, LOW);

  pinMode(5, OUTPUT);
  gimSerial.begin(baudrate);

  ioSerial.begin(baudrate);
  ioSerial.println("Ready.");
}

void loop() {
  if (gimSerial.available() > 0) {
    char c = gimSerial.read();
    ioSerial.print(c);
  }

  if (ioSerial.available() > 0) {
    char c = ioSerial.read();
    ioSerial.print(c);    // Echo back
    gimSerial.print(c);
  }  
}
