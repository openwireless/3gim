/*
 * MMA8451Q Sample sketch for CAIM -- Read 3-axis accelarations without FIFO
 */

#include <Wire.h>
#include "mma8451q.h"

// Serial monitor port
#define monSerial     SerialUSB

// Initial settings
#define XYZ_DATA_CFG  (m8XYZ_DATA_FS_2G)
#define CTRL_REG1     (m8CTRL_REG1_DR_100HZ | m8CTRL_REG1_LNOISE | m8CTRL_REG1_ACTIVE)
#define CTRL_REG2     (m8CTRL_REG2_MODE_HIGH_RES)
#define CTRL_REG3     (0)
#define CTRL_REG4     (0)
#define CTRL_REG5     (0)

#define MG_PER_LSB    0.244		// Conversion factor from LSB to mg [mg/LSB]

MMA8451Q  acc(LOW);

void setup() {
  Wire.begin();
  
  monSerial.begin(115200);
  while (! monSerial)
    ;

  // Check id
  uint8_t id = acc.readRegister(m8REG_WHO_AM_I);
  if (id != m8WHO_AM_I_MMA8451Q_ID) {
    monSerial.print("Bad id=0x");
    monSerial.println(id, HEX);
    while (1) ;
  }

  // Reset MMA8451Q
  acc.writeRegister(m8REG_CTRL_REG2, m8CTRL_REG2_RST);
  while (acc.readRegister(m8REG_CTRL_REG2) & m8CTRL_REG2_RST)
    ;   // wait for ready

  // Set up MMA8451Q
  acc.writeRegister(m8REG_XYZ_DATA_CFG, XYZ_DATA_CFG);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG2, CTRL_REG2);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG3, CTRL_REG3);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG4, CTRL_REG4);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG5, CTRL_REG5);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG1, CTRL_REG1);    // and Go!  
}

void loop() {
  uint8_t status = acc.readRegister(m8REG_STATUS);
  if (status & m8STATUS_ZYXD) {
    uint8_t data[6];
    acc.readMultiRegisters(m8REG_OUT_X_MSB, 6, data);
    int ax = acc.toRaw(data[0], data[1]) * MG_PER_LSB;
    int ay = acc.toRaw(data[2], data[3]) * MG_PER_LSB;
    int az = acc.toRaw(data[4], data[5]) * MG_PER_LSB;
    char  line[60];
    sprintf(line, "%d,%d,%d", ax, ay, az);
    monSerial.println(line);
  }
  delayMicroseconds(100);
}
