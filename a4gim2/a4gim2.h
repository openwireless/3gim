//***********************************************************
//	a4gim2.h -- Arduino Control Library for 4GIM(V1) for Mega/Zero/m0..
//
//	History:
//		R1.0 2018/09/02  1st Release for 4GIM(Ver1.0)
//
//	Author:
//		Open wireless Alliance and Atsushi Daikoku
//
//	Notes:
//		Lower compatible with Arduino GSM/GPRS Shield library.
//		Notices as bellow:
//		- Use HardwareSerial
//		- Use specified power control or not(always on).
//*** ***********************************************

#ifndef _A4GIM2_H_
#define _A4GIM2_H_  

#include <Arduino.h>

/*
	Define constants
*/
//	define a4gserial as hardware serial port
#define	a4gsSerial					Serial		// Use serial port

//	for compatibility of GSM.h
#define	ctrlz						26			// Ascii character for ctr+z. End of a SMS.
#define	cr							13			// Ascii character for carriage return. 
#define	lf							10			// Ascii character for line feed. 

//	Basic constants
#define	a4gsBAUDRATE				115200		// Default UART baudrate if "a4gsBAUDRATE" is undefined(3GIM default baudrate is 9600)
#define	a4gsDATE_SIZE				11			// date space size ("YYYY/MM/DD\0" format, used in getTime(), included '\0')
#define	a4gsTIME_SIZE				9			// time space size ("HH:MM:SS\0" format(hour is 24h-way), used in getTime(), included '\0')
#define	a4gsCS_ASCII				0			// SMS Text Char Code: ASCII (used in sendSMS())
#define	a4gsCS_UNICODE				1			// SMS Text Char Code: UNICODE(Little Endian) (used in sendSMS()) --@Not used R4.0
#define	a4gsIMEI_SIZE				16			// imei space size ("99..9\0" format, used in getIMEI(), included '\0')
#define	a4gsDEFAULT_PORT			0			// Default port number(for httpGET() and httpPOST())

//	Return values in general
#define	a4gsSUCCESS					0
#define	a4gsERROR					(-1)

//	Maximum lengths
#define	a4gsMAX_VERSION_LENGTH		5			// Maximum bytes of Version number(used in getVersion())
#define	a4gsMAX_MSN_LENGTH			11			// Maximum bytes of Phone Number in Japan(used in readSMS())
#define	a4gsMAX_HOST_LENGTH			96			// Maximum length of host name --@change R4.0
#define	a4gsMAX_DATA_LENGTH			1024		// Maximum length of data(at read/write)
#define	a4gsMAX_TUNNEL_DATA_LENGTH	32000		// Maximum length of data in writeBegin() --@add R4.0

#define	a4gsMAX_URL_LENGTH			192			// Maximum length of URL(used in httpGET() and httpPOST())
#define	a4gsMAX_RESULT_LENGTH		1024		// Maximum length of result(used in httpGET() and httpPOST())
#define	a4gsMAX_HEADER_LENGTH		512			// Maximum length of header(used in httpPOST())
#define	a4gsMAX_BODY_LENGTH			1024		// Maximum length of header(used in httpPOST())
#define	a4gsMAX_TWEET_LENGTH		140			// Maximum length of Tweet message(used in tweet())

//	Return values of getService()
#define	a4gsSRV_NO					0			// Out of service
#define	a4gsSRV_PS					1			// Data(packet) only
#define	a4gsSRV_CS					2			// Voice only -- @Not used R4.0
#define	a4gsSRV_BOTH				3			// Data and voice both -- @Not used R4.0

/*
	Declare class
*/
class A4GS
{
  public:
  	A4GS() : _powerPin(0) { };
	enum a4gs_st_e { ERROR, IDLE, READY, TCPCONNECTEDCLIENT };

	// compatible methods with Arduino GSM/GPRS Shield library
	int getStatus() { return _status; };
	int begin(char* pin = 0);
	int begin(char* pin, uint32_t baudrate);
	int end(void);
	int restart(int pin = 0);
	int start(int pin = 0);
	int shutdown(void);
		//-- This library do not use "pin" parameter, so ignore it.
	int getIMEI(char* imei);
	int connectTCP(const char* server, int port);
	int disconnectTCP(void);
	int getStatusTCP(int *status, int *tcpnotif, long *remainedBytes, long *receivedBytes);	//@add R4.0
	int setTCPParams(int timeout1, int timeout2);	//@add R4.3 -- use only 3GIM(V2.2) above
	int write(uint8_t c);	// support binary data
	int write(const char* str);	// doesn't support binary data
	int write(const uint8_t* buffer, size_t sz);	// support binary data
	int writeBegin(size_t sz);
	int read(char* result, int resultlength);	// non-blocking I/O and doesn't support binary data
	int read(void);		// non-blocking I/O and support binary data
	int read(uint8_t* buffer, size_t sz);	// non-blocking I/O and support binary data
		//-- This library do not support tcp/ip server functions.
	int httpGET(const char* server, uint16_t port, const char* path, char* result, int resultlength, boolean ssled = false, const char* header = NULL);
	int httpPOST(const char* server, uint16_t port, const char* path, const char* header, const char* body, char* result, int* resultlength, boolean ssled = false);
			//-- httpPOST() is not compatible parameters with Arduino GSM/GPRS Shield library
	int tweet(const char* token, const char* msg);
		//-- tweet() sends a message to twitter with the text "msg"and the token "token". 
		//-- Get the token from http://arduino-tweet.appspot.com/
	int beginUDP(const char* server, int port);
	int endUDP(void);
	int sendUDP(char *str);		// send string
	int sendUDP(uint8_t data[], size_t sz);	// send binary data
	int getStatusUDP(int *status);
	//-- Extended methods --//
	int getServices(int& status);
	int getRSSI(int& rssi);
	int getTime(char* date, char* time);
	int getTime2(uint32_t& seconds);
	int getVersion(char *version);
	int setDefaultProfile(const char *apn, const char *user, const char *password);	//@change R4.0
	int getDefaultProfile(char *apn, char *user, char *password);	//@change R4.0
	int setBaudrate(uint32_t baudrate);
	int setLED(boolean sw);
	int setAirplaneMode(boolean sw);
	int enterAT(uint32_t duration);
	int getResult(char *buf, int *len, uint32_t timeout);
	
  private:
	int _status;		// for Compatible with GSM.h
	int _powerPin;
	void sendCommand(const char* cmd);
	void sendData(const char* data);
	void discardUntil(const char match);
};

extern	A4GS	a4gs;	// Define the instance of A4GS

#endif // _A4GIM2_H_
