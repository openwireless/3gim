/*
 * MMA8451Q Sample sketch for CAIM -- Read 3-axis accelarations with FIFO
 */

#include <Wire.h>
#include "mma8451q.h"

// monSerial monitor port
#define monSerial     SerialUSB

// Initial settings
#define XYZ_DATA_CFG  (m8XYZ_DATA_FS_2G)
#define F_SETUP       (m8F_SETUP_F_MODE_FILL_BUF)
#define CTRL_REG1     (m8CTRL_REG1_DR_100HZ | m8CTRL_REG1_LNOISE)
#define CTRL_REG2     (m8CTRL_REG2_MODE_HIGH_RES)

#define N_SAMPLES     10        // Sample number read at once
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
    while (1) ;     // Stop here
  }

  monSerial.print("id=0x");
  monSerial.println(id, HEX);

  // Reset MMA8451Q
  acc.writeRegister(m8REG_CTRL_REG2, m8CTRL_REG2_RST);
  while (acc.readRegister(m8REG_CTRL_REG2) & m8CTRL_REG2_RST)
    ;   // wait for ready

  // Set up MMA8451Q
  acc.writeRegister(m8REG_CTRL_REG1, CTRL_REG1);  
  delay(10);
  acc.writeRegister(m8REG_XYZ_DATA_CFG, XYZ_DATA_CFG);
  delay(10);
  acc.writeRegister(m8REG_F_SETUP, F_SETUP);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG2, CTRL_REG2);
  delay(10);
  uint8_t reg1 = acc.readRegister(m8REG_CTRL_REG1);
  acc.writeRegister(m8REG_CTRL_REG1, (reg1 | m8CTRL_REG1_ACTIVE));
}

void loop() {
  if (acc.getFIFOEntries() >= N_SAMPLES) {
    int samples[N_SAMPLES];
    int nSamples = acc.readFIFO(samples, N_SAMPLES);
    for (int j = 0, i = 0; j < nSamples; j++, i += 3) {
      char  line[60];
	  int ax = samples[i]*MG_PER_LSB;
	  int ay = samples[i+1]*MG_PER_LSB;
	  int az = samples[i+2]*MG_PER_LSB;
      sprintf(line, "%d,%d,%d", ax, ay, az);
      monSerial.println(line);
    }
  }
  delayMicroseconds(500);
}
