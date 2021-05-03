/*
 *  mgim.h
 *
 *  Control library for MGIM(LTE-M IoT Module)
 *
 *  R0  2020/03/21 (A.D)
 *  R1  2021/02/20 (A.D) fix typo "setAccelerationHandler"
 *  R2  2021/02/23 (A.D) add argument "mode" to setAccelerationHandler()
 *  R3  2021/04/25 (A.D) change for mgim(V4.1)
 *
 *  Copyright(c) 2020-2021 TABrain Inc. All rights reserved.
 */

#ifndef _mgim_h_
#define _mgim_h_

#include <Arduino.h>

//-- On board devices
#define mgSTTS751_I2C_ADDDR         0b0111011   // I2C address of stts751 on board
#define mgSERIAL_MONITOR            SerialUSB   // Serial monitor port
#define mgSERIAL_EXTERNAL           Serial1     // External Serial port

//-- Error codes
  // Succeed(No error)
#define mgSUCCESS                   0           // When the call is successful
  // common errors
#define mgERR_BAD_PARAM             100         // Bad parameters
#define mgERR_ERROR                 101         // Unknown error

//-- Pin definitions
const int _mgLedPin                 = 7;        // [out] On board Led (On:HIGH/Off:LOW)
const int _mgINT1Pin                = 6;        // [in] LIS2DWTR interrupt #1 (Active:LOW)
const int _mgHL7800PowerPin         = 4;        // [out] HL7800 power control (On:HIGH/Off:LOW)
const int _mgVinPin                 = A5;       // [in] VIN * 1/2
const int _mgHL7800WakeUpPin        = 2;        // [out] HL7800 WAKEUP (Active:HIGH)
const int _mgHL7800ResetPin         = 5;        // [out] HL7800 RESET_IN_N (Active:LOW)
const int _mgHL7800PowerOnPin       = 38;       // [out] HL7800 PWR_ON_N (Active:LOW)
const int _mgHL7800VGPIOPin         = A3;       // [out] HL7800 VGPIO (1.8V:On/0V:Off)
const int _mgHL7800ShutdownPin      = 22;       // [out] HL7800 Shutdown (Active:LOW)
const int _mgHL7800CTS              = 23;       // [in] HL7800 UART CTS
const int _mgHL7800RTS              = 24;       // [out] HL7800 UART RTS

//-- Class definition
class MGIM {
  public:
      const int externalPins[11] = { 0, 8, 9, 10, 11, 12, 13, A0, A1, A2, A4 }; // expect D1(TX External)
      const int numberOfExternalPins = sizeof(externalPins) / sizeof(int);

      MGIM() { }

      int begin(void);
      void setLed(int onOff);
      void setAccelerationHandler(void (*handler)(void), PinStatus mode = LOW);
      int getVIN(void);

  private:
      //
};

// Declare MGIM instance
extern MGIM  mgim;

#endif // _mgim_h_
