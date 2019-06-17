/*
 *  sgim.h -- SGIM Control Class Library (Interface)
 *
 *  History:
 *      R1.0 2019/06/17  1st Release for SGIM(Ver1.0)
 *
 *  Author:
 *      Open wireless Alliance and 605e
 *
 */

#ifndef _SGIM_H_
#define _SGIM_H_

#include <Arduino.h>

// Symbols
 // for Sigfox
#define sgSerial            Serial
#define BAUDRATE_SIGFOX     9600UL
#define MAX_PAYLOAD_SIZE    12
#define TIMEOUT_LOCAL       3000UL
#define TIMEOUT_UPLINK      10000UL
#define TIMEOUT_DOWNLINK    60000UL     //  Wait up to 60 seconds for response from SIGFOX module. Includes downlink response.

// Constants
const int ledPin            = 5;
const int interruptPin      = 6;
const int sigfoxStatusPin   = 7;
const int sigfoxResetPin    = 8;
const int sigfoxCPULedPin   = 9;
const int vbatPin           = A5;
const uint8_t markerPosMax  = 5;

// Class

class SGIM {
  public:
    SGIM(void) {
        _zone = 3;      // RZ3(Japan)
        _lastSent = 0;
    };
    
    // Control functions
    bool begin(void);
    bool end(void);
    bool reboot(void);  // Reboot Sigfox module
    void reset(void);   // Reset SAMD21G18

    // On board functions
    void setLed(int onoff);
    int getVBAT(void);
    int onDetectMotion(void (*func)(void));
    
    // ID and PAC functions
    bool getID(String &id);
    bool getPAC(String &pac);
    
    // Message passing functions
    bool sendMessage(const String &payload);
    bool sendMessageAndGetResponse(const String &payload, String &response);
    bool isReady(void);
    
    // AT command functions
    bool enterCommandMode(void);
    bool exitCommandMode(void);

    // Sleep functions
    bool sleep(void);
    bool wakeup(void);
    bool isActive(void);
    
    // Misc functions
    bool getVoltage(int &voltage);  // [mV]
    bool getTemperature(float &temperature);

    // Message conversion functions
    String toHex(uint8_t u8);
    String toHex(uint16_t u16);
    String toHex(uint32_t u32);
    String toHex(char *s);
    String toHex(float f);

  private:
    int _zone;          //  1 to 4 representing SIGFOX frequencies RCZ 1 to 4. (RCZ3)
    uint32_t _lastSent;
    uint8_t _markerPos[markerPosMax];
    bool _hasInitialized = false;
    void (*_onMotionHandler)(void) = NULL;

    bool sendBuffer(const String &buffer, const uint32_t timeout, uint8_t expectedMarkerCount, String &response, uint8_t &actualMarkerCount);
    bool sendCommand(const String &cmd, uint8_t expectedMarkerCount, String &result, uint8_t &actualMarkerCount);
};

extern SGIM     sgim;

#endif // _SGIM_H_
