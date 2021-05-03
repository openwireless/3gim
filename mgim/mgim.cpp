/*
 *  mgim.cpp
 *
 *  Control library for MGIM(LTE-M IoT Module)
 *
 *  R0  2020/03/21 (A.D)
 *  R1  2021/02/20 (A.D) fix typo "setAccelerationHandler"
 *  R3  2021/04/25 (A.D) change for mgim(V4.1)
 *
 *  Copyright(c) 2020-2021 TABrain Inc. All rights reserved.
 */

#include "mgim.h"

// Symbols
#define VIN_COUNT            10              // getVIN()で供給電圧を計測する回数

// Define MGIM instance
MGIM    mgim;

/**
 *  @fn
 *
 *  MGIMボードおよびライブラリを初期化する
 *
 *	@param			    なし
 *  @return             0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int MGIM::begin(void) {
    // Turn off led on board
    pinMode(_mgLedPin, OUTPUT);
    digitalWrite(_mgLedPin, LOW);           // Turn off led
    // turn off hl7800
    pinMode(_mgHL7800PowerOnPin, OUTPUT);
    digitalWrite(_mgHL7800PowerOnPin, LOW); // Deactive PWR_ON_N
    pinMode(_mgHL7800WakeUpPin, OUTPUT);
    digitalWrite(_mgHL7800WakeUpPin, LOW);  // Deactive WAKEUP
    pinMode(_mgHL7800ResetPin, OUTPUT);
    digitalWrite(_mgHL7800ResetPin, LOW);   // Deactive RESET_IN_N
    pinMode(_mgHL7800PowerPin, OUTPUT);
    digitalWrite(_mgHL7800PowerPin, LOW);   // HL7800 power off

    // Set up interrupt pin for LIS2WD
    pinMode(_mgINT1Pin, INPUT);               // fix @R2

    return (mgSUCCESS);
}

/**
 *  @fn
 *
 *  MGIMボード上のLEDをOn/Offする
 *
 *	@param(onOff)       On(1)またはOff(0)
 *  @return             なし
 *  @detail
 */
void MGIM::setLed(int onOff) {
    if (onOff)
        digitalWrite(_mgLedPin, HIGH);            // Turn on
    else
        digitalWrite(_mgLedPin, LOW);             // Turn off
}

/**
 *  @fn
 *
 *  MGIMボード上の加速度センサの割り込みハンドラを設定・解除する
 *
 *	@param(handler)     ハンドラ関数へのポインタ(0以外)、または解除(0)
 *  @param(mode)        割り込みモード(デフォルトでは LOW)
 *  @return             なし
 *  @detail
 */
void MGIM::setAccelerationHandler(void (*handler)(void), PinStatus mode) {
    if (handler == NULL)
        detachInterrupt(_mgINT1Pin);
    else
        attachInterrupt(_mgINT1Pin, handler, mode);
}

/**
 *  @fn
 *
 *  MGIMボードへの供給電圧を計測する
 *
 *	@param              なし
 *  @return             計測した電圧[mV]
 *  @detail             電圧は10ミリ秒間隔でVIN_COUNT回数だけ計測し、平均値をとる
 */
int MGIM::getVIN(void) {
    int total = 0;
    for (int i = 0; i < VIN_COUNT; i++) {
        total += analogRead(_mgVinPin);
        delay(10);
    }

    return ((total * 2 * 3.226) / VIN_COUNT);
}

// End of mgim.cpp
