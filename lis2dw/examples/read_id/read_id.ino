/*
 * LIS2DW Sample sketch for CAIM -- Read "Who am I"
 */

#include <Wire.h>
#include "lis2dw.h"

// Serial monitor port
#define SERIAL_MONITOR     SerialUSB

LIS2DW  acc(HIGH);

void setup() {
  Wire.begin();

  SERIAL_MONITOR.begin(9600);
  while (! SERIAL_MONITOR)
    ;
  SERIAL_MONITOR.println("Try to read ID from the device.");

  uint8_t id = acc.readRegister(s2REG_WHO_AM_I);

  SERIAL_MONITOR.print("id=0x");
  SERIAL_MONITOR.println(id, HEX);

  Wire.end();
}

void loop() {
}
