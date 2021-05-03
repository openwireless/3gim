/*
 *	LIS2DW.cpp
 *
 *	LIS2DWQ control library for Arduino (Implementation)
 *
 *	Version:
 *		R1.0  2021.04.25
 *
 *	Note:
 *		LIS2DW is a 14bit ADC 3-Axis MEMS Accelerometer made by STmaicro.
 *
 *	Copyright(c) 2021 TABrain Inc.
 */

#include "lis2dw.h"

/*
 *	Public methods
 */

LIS2DW::LIS2DW(int sa0) {
	if (sa0)
		_i2cAddress = 0x19;
	else
		_i2cAddress = 0x18;
}

uint8_t LIS2DW::readRegister(uint8_t reg) {
	Wire.beginTransmission(_i2cAddress);
	Wire.write(reg);
	Wire.endTransmission(false); 	// uses repeated start
    Wire.requestFrom(_i2cAddress, (uint8_t)1);
    if (! Wire.available())
		return (0xff);		// error
    return (Wire.read());
}

void LIS2DW::readMultiRegisters(uint8_t reg, int bytes, uint8_t data[]) {
	uint8_t reg2 = readRegister(s2REG_CTRL2);

	Wire.beginTransmission(_i2cAddress);
    Wire.write(s2REG_CTRL2);
	Wire.write(reg2 | s2CTRL2_IF_ADD_INC);
    Wire.endTransmission();

	Wire.beginTransmission(_i2cAddress);
    Wire.write(reg); 	// 0x80 for autoincrement
    Wire.endTransmission();

    Wire.requestFrom(_i2cAddress, (uint8_t)bytes);
	while (bytes-- > 0) {
		if (! Wire.available())
			*data++ = 0xff;			// read error
		else
			*data++ = Wire.read();
	}

	Wire.beginTransmission(_i2cAddress);
    Wire.write(s2REG_CTRL2);
	Wire.write(reg2 & ~s2CTRL2_IF_ADD_INC);
    Wire.endTransmission();
}

void LIS2DW::writeRegister(uint8_t reg, uint8_t data) {
	Wire.beginTransmission(_i2cAddress);
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission();
}

int LIS2DW::getAccelerations(int *ax, int *ay, int *az) {
	uint8_t data[6];
	for (int i = 0; i < 6; i++)
		data[i] = 0;
	readMultiRegisters(s2REG_OUT_X_L, 6, data);
	*ax = toRaw(data[1], data[0]);
	*ay = toRaw(data[3], data[2]);
	*az = toRaw(data[5], data[4]);

	return 0;	// OK
}

int LIS2DW::readFIFO(int samples[], int numSamples) {
	int m, n;
	for (m = n = 0; m < numSamples; n += 3, m++) {
		Wire.beginTransmission(_i2cAddress);
		Wire.write(s2REG_OUT_X_L);
		Wire.endTransmission(false); 	// uses repeated start
		Wire.requestFrom(_i2cAddress, (uint8_t)6);
		int16_t low = Wire.read() & 0xff;
		int16_t high =  (int16_t)Wire.read();
		samples[n] = ((high << 8) + low) >> 2;		// X-axis
		low = Wire.read() & 0xff;
		high = (int16_t)Wire.read();
		samples[n+1] = ((high << 8) + low) >> 2;	// Y-axis
		low = Wire.read() & 0xff;
		high = (int16_t)Wire.read();
		samples[n+2] = ((high << 8) + low) >> 2;	// Z-axis
	}

	return (m);		// return number of read samples
}

// End of "lis2dw.cpp"
