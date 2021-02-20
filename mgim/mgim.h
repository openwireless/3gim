/*
 *  mgim.h
 *
 *  Control library for MGIM(LTE-M IoT Module)
 *
 *  R0  2020/03/21 (A.D)
 *  R1  2021/02/20 (A.D) setAccelerationHandler()の綴りを修正
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#ifndef _mgim_h_
#define _mgim_h_

#include <Arduino.h>

//-- On board devices
#define mgSTTS751_I2C_ADDDR         0b0111011    // I2C address of stts751 on board
#define mgSERIAL_MONITOR            SerialUSB    // Serial monitor port
#define mgSERIAL_EXTERNAL           Serial1      // External zwrial port

//-- Error codes
  // Succeed(No error)
#define mgSUCCESS                   0            // When the call is successful
  // common errors
#define mgERR_BAD_PARAM             100          // Bad parameters
#define mgERR_ERROR                 101          // Unknown error

//-- Class definition
class MGIM {
  public:
      const int externalPins[11] = { 0, 8, 9, 10, 11, 12, 13, A0, A1, A2, A4 }; // expect D1(TX External)
      const int numberOfExternalPins = sizeof(externalPins) / sizeof(int);

      MGIM() { }

      int begin(void);
      void setLed(int onOff);
      void setAccelerationHandler(void (*handler)(void));
      void powerOnExternal(void);
      void powerOffExternal(void);
      int getVIN(void);

  private:
      // Pin definitions
      const int _ledPin = 7;            // [out] On board Led (Active:HIGH)
      const int _powerExternalPin = 6;  // [out] External power on (Active:HIGH)
      const int _hl7800PowerOnPin = 4;  // [out] HL7800 Power on (Active:HIGH)
      const int _hl7800WakeUp = 3;      // [out] HL7800 Wake up (Active:HIGH)
      const int _hl7800Reset = 2;       // [out] HL7800 Reset (Active:HIGH)
      const int _hl7800Vgpio = A3;      // [out] HL7800 VGPIO
      const int _int1Pin = 5;           // [in] MMA8451Q Interrupt #1 (Active:LOW)
      const int _vinPin = A5;           // [in] VIN * 1/2
};

// Declare MGIM instance
extern MGIM  mgim;

#endif // _mgim_h_
