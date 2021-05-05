/*
 *	stts751.cpp - Digital temperature sensor STTS751 control library
 *
 *  Digital temperature sensor STTS751 control library
 *
 *  R0.1  2020/02/29 (605e)
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#include <Wire.h>
#include "stts751.h"

/**
 *  @fn
 *
 *  stts751を初期化する
 *
 *	@param			    なし
 *  @return             なし
 *  @detail             本関数の呼び出し以降、getTemperature()を呼び出すことで温度が計測できるようになる
 */
void STTS751::begin(void) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(0x03);
    Wire.write(0b10001100);
    Wire.endTransmission();
	Wire.write(0x04);
    Wire.write(0x04);
    Wire.endTransmission();
}

/**
 *  @fn
 *
 *  温度を計測する
 *
 *	@param			    なし
 *  @return             計測した温度(摂氏)を10倍した値
 *  @detail             なし
 */
int STTS751::getTemperature(void) {
	uint32_t data[2];
	Wire.beginTransmission(_i2cAddress);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(_i2cAddress, 1);
	if (Wire.available() == 1)
		data[0] = Wire.read();
    Wire.beginTransmission(_i2cAddress);
    Wire.write(0x02);
    Wire.endTransmission();
    Wire.requestFrom(_i2cAddress, 1);
    if (Wire.available() == 1)
		data[1] = Wire.read();

    long temp = ((data[0] << 8) + (data[1] & 0xFC)) / 16;
    if (temp > 2047)
		temp -= 4096;

	return (temp * 0.625);
}

// End of stts751.cpp
