/*
 *	stts751.h
 *
 *  Digital temperature sensor STTS751 control library
 *
 *  R0.1  2020/02/29 (605e)
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#ifndef	_STTS751_H_
#define	_STTS751_H_

#include <Arduino.h>

// Symbols
#define s7STTS751_I2C_ADDDR  	  0b0111011       // Default i2c address

class STTS751 {
  public:
    STTS751(uint8_t i2cAddress = s7STTS751_I2C_ADDDR): _i2cAddress(i2cAddress) { }
	void begin(void);
	int getTemperature(void);

private:
    uint8_t _i2cAddress;
};

#endif // _STTS_751_h_
