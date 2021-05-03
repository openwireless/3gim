/*
 *	MMA8451Q.cpp
 *
 *	MMA8451Q control library for Arduino (Implementation)
 *
 *	Version:
 *		R1.0  2017.01.09
 *		R1.1  2017.02.01  add getAccelerations()
 *		R1.2  2020.01.01  fix getFIFOEntries() and readFIFO() return wrong number
 *
 *	Note:
 *		MMA8451Q is a 14bit ADC 3-Axis MEMS Accelerometer.
 *
 *	Copyright(c) 2017 A.Daikoku
 */

#include "mma8451q.h"

/*
 *	Public methods
 */

MMA8451Q::MMA8451Q(int sa0) {
	if (sa0)
		_i2cAddress = 0x1d;
	else
		_i2cAddress = 0x1c;
}

uint8_t MMA8451Q::readRegister(uint8_t reg) {
	Wire.beginTransmission(_i2cAddress);
	Wire.write(reg);
	Wire.endTransmission(false); 	// uses repeated start
    Wire.requestFrom(_i2cAddress, (uint8_t)1);
    if (! Wire.available())
		return 0xff;		// error
    return (Wire.read());
}

void MMA8451Q::readMultiRegisters(uint8_t reg, int bytes, uint8_t data[]) {
	Wire.beginTransmission(_i2cAddress);
	Wire.write(reg);
	Wire.endTransmission(false); 	// uses repeated start
    Wire.requestFrom(_i2cAddress, (uint8_t)bytes);
	while (bytes-- > 0) {
		if (! Wire.available())
			*data++ = 0xff;			// read error
		else
			*data++ = Wire.read();
	}
}

void MMA8451Q::writeRegister(uint8_t reg, uint8_t data) {
	Wire.beginTransmission(_i2cAddress);
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission();
}

int MMA8451Q::getAccelerations(int *ax, int *ay, int *az) {
	uint8_t data[6];
	for (int i = 0; i < 6; i++)
		data[i] = 0;
	readMultiRegisters(m8REG_OUT_X_MSB, 6, data);
	*ax = toRaw(data[0], data[1]);
	*ay = toRaw(data[2], data[3]);
	*az = toRaw(data[4], data[5]);

	return 0;	// OK
}

int MMA8451Q::readFIFO(int samples[], int numSamples) {
	int m, n;
	for (m = n = 0; m < numSamples; n += 3, m++) {
		Wire.beginTransmission(_i2cAddress);
		Wire.write(m8REG_OUT_X_MSB);
		Wire.endTransmission(false); 	// uses repeated start
		Wire.requestFrom(_i2cAddress, (uint8_t)6);
		int high = (int)Wire.read() << 6;
		samples[n] = high + ((Wire.read() & 0xfc) >> 2);	// X
		if (high & 0x2000)
			samples[n] |= 0xffffc000;
		high = (int)Wire.read() << 6;
		samples[n+1] = high + ((Wire.read() & 0xfc) >> 2);	// Y
		if (high & 0x2000)
			samples[n+1] |= 0xffffc000;
		high = (int)Wire.read() << 6;
		samples[n+2] = high + ((Wire.read() & 0xff) >> 2);	// Z
		if (high & 0x2000)
			samples[n+2] |= 0xffffc000;
	}

	return (m);		// return number of read samples
}

// End of "mma8451q.cpp"
