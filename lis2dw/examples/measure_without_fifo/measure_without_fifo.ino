/*
 * LIS2DW Sample sketch for CAIM -- Read 3-axis accelarations without FIFO
 */

#include <Wire.h>
#include "lis2dw.h"

// Serial monitor port
#define SERIAL_MONITOR     SerialUSB

// Initial settings
#define REG_CTRL1     (s2ODR_200 | s2MODE_HIGH_PERFORMANCE)
#define REG_CTRL2     (s2CTRL2_BDU)
#define REG_CTRL6     (s2FS_2G | s2CTRL6_LOW_NOISE | s2BW_FILT_DIV2 | s2CTRL6_FDS)

LIS2DW  acc(HIGH);

void setup() {
  Wire.begin();

  SERIAL_MONITOR.begin(115200);
  while (! SERIAL_MONITOR)
    ;

  // Check id
  uint8_t id = acc.readRegister(s2REG_WHO_AM_I);
  if (id != s2WHO_AM_I_ID) {
    SERIAL_MONITOR.print("Bad id=0x");
    SERIAL_MONITOR.println(id, HEX);
    while (1) ;
  }

  // Reset LIS2DW
  acc.writeRegister(s2REG_CTRL2, s2CTRL2_SOFT_RESET);
  while (acc.readRegister(s2REG_CTRL2) & s2CTRL2_SOFT_RESET)
    ;   // wait for ready

  // Set up LIS2DW
  acc.writeRegister(s2REG_CTRL1, REG_CTRL1);
  delay(10);
  acc.writeRegister(s2REG_CTRL2, REG_CTRL2);
  delay(10);
  acc.writeRegister(s2REG_CTRL6, REG_CTRL6);
  delay(10);
}

void loop() {
  uint8_t status = acc.readRegister(s2REG_STATUS);
  if (status & s2STATUS_DRDY) {
    uint8_t data[6];
    acc.readMultiRegisters(s2REG_OUT_X_L, 6, data);
    int ax = acc.tomg(acc.toRaw(data[1], data[0]), s2FS_2G);
    int ay = acc.tomg(acc.toRaw(data[3], data[2]), s2FS_2G);
    int az = acc.tomg(acc.toRaw(data[5], data[4]), s2FS_2G);
    char  line[60];
    sprintf(line, "%d,%d,%d", ax, ay, az);
    SERIAL_MONITOR.println(line);
  }
  delayMicroseconds(100);
}
