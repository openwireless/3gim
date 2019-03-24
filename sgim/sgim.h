/*
 *	sgim.h -- SGIM(V1) Control Class Library (Interface)
 *
 *	History:
 *		R0.1 2018/11/24  1st Release for SGIM(Ver1.0)
 *      R0.2 2019/01/07  fix sleep(), add isActive()
 *      R0.3 2019/03/09  fix sendBuffer()
 *      R0.4 2019/03/21  add decode functions
 *
 *	Author:
 *		Open wireless Alliance and @605e
 *
 */

#ifndef _SGIM_H_
#define _SGIM_H_

#include <Arduino.h>

// Symbols
#define	sgSerial            Serial
#define sgBAUDRATE_SIGFOX   9600UL
#define sgMAX_PAYLOAD_SIZE  12
#define	sgTIMEOUT_LOCAL     3000UL
#define sgTIMEOUT_UPLINK    10000UL
#define	sgTIMEOUT_DOWNLINK  60000UL		//  Wait up to 60 seconds for response from SIGFOX module.  Includes downlink response.

// Constants
const int ledPin 			= 5;    // [OUT] On(HIGH)/Off(LOW)
const int interruptPin 		= 6;    // [IN] Interrupt on LOW 
const int sigfoxStatusPin   = 7;    // [IN]
const int sigfoxResetPin    = 8;    // [OUT]
const int sigfoxCPULedPin   = 9;    // [IN] Active(HIGH)/Sleep(LOW)
const int vbatPin 			= A5;   // [ADC] 
const uint8_t markerPosMax 	= 5;    // 

// Class

class SGIM {
  public:
    SGIM(void) {
		_zone = 3;		// RZ3(Japan)
		_lastSent = 0;
	};
	
	// Control functions
	bool begin(void);
	bool end(void);
	bool rebootSigfoxModule(void);	// Reboot Sigfox module
	void resetBoard(void);	// Reset SAMD21G18

	// On board functions
	void setLed(int onoff);
	int onDetectMotion(void (*func)(void));
	
	// ID and PAC functions
	bool getID(String &id);
	bool getPAC(String &pac);
	
	// Message passing functions
	bool sendMessage(const String &payload);
	bool sendMessageWithResponse(const String &payload, String &response);
	bool readyToSend(void);
	
	// AT command functions -- not implemented
	bool enterCommandMode(void);
	bool exitCommandMode(void);

	// Power management functions
	bool sleep(void);
	bool wakeup(void);
	bool isActive(void);
	
	// Misc functions(provided by Sigfox module)
	bool getVoltage(int &voltage);  // [mV]
	bool getTemperature(float &temperature);

	// Message encode/decode functions
	String toHex(uint8_t u8);
	String toHex(uint16_t u16);
	String toHex(uint32_t u32);
	String toHex(char *s);
	String toHex(float f);
    uint8_t toUint8(String s);
    uint16_t toUint16(String s);
    uint32_t toUint32(String s);

  private:
	bool _hasInitialized = false;
	int _zone;			//  1 to 4 representing SIGFOX frequencies RCZ 1 to 4. (RCZ3)
	uint32_t _lastSent;
	uint8_t _markerPos[markerPosMax];
	void (*_onMotionHandler)(void) = NULL;

	bool sendBuffer(const String &buffer, const uint32_t timeout, uint8_t expectedMarkerCount, String &response, uint8_t &actualMarkerCount);
	bool sendCommand(const String &cmd, uint8_t expectedMarkerCount, String &result, uint8_t &actualMarkerCount);
    uint8_t toHexdecimal(char xdigit);
};

extern SGIM		sgim;

#endif // _SGIM_H_
