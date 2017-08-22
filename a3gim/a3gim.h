//***********************************************************
//	a3gim.h -- Arduino Control Library for 3GIM
//
//	History:
//		R1.0 2012/10/06  1st Release for IEM 3G Shiled(Ver1.0)
//		R1.1 2012/10/14  2nd Release(Operate by few memories)
//						 (modify sendSMS(),onSMSReceived(),httpGET(),httpPOST(),tweet(),connectTCP())
//		R1.2 2012/10/28  Bug fix (httpGET(), httpPOST(), tweet(), availableSMS() and redSMS())
//		R1.3 2013/01/01  Support "https" in httpGET() and httpPost()
//		R1.4 2013/04/16  Bug fix (discardUntil() has never return)
//		R2.0 2013/07/10  Change interface of TCP/IP and discardUntil(), some bugs fix
//						 a3gsMAX_TWEET_LENGTH set as 60 (bytes)
//		R2.1 2013/08/17  Buf fix (discardUntil() has never return)
//		R2.2 2013/10/27  Bug fix (read(res, reslength))
//		R2.3 2014/07/06  Support binary data in read() and add new version read() function, bug fix write()
//		R3.0 2014/08/30  Support for gw3g R2.0 and add follow functions:
//						   setAirplaneMode(), put(), get()
//		R3.0 2014/12/10  Support 3GIM, rename as a3gim.h
//		R3.1 2015/03/22  Tested to 3GIM(V1.1)
//		                           Support updateProfile() and encryptProfile()
//		R4.0 2016/02/22  Support 3GIM(V2) only
//		R4.1 2016/05/15  Bug fix enterAT()
//		R4.3 2017/08/16  Support 3GIM(V2.2): Remove SMS functions, add the following functions
//							setLocationParams(), setTCPParams()
//                       Bug fixed the following functions,
//							setDefaultProfile(), enterAT()
//
//	Author:
//		Open wireless Alliance(Old 3G Shield Alliance) and Atushi Daikoku
//
//	Notes:
//		Lower compatible with Arduino GSM/GPRS Shield library.
//		Notices as bellow:
//		- Use SoftwareSerial library (RxD is D4, TxD is D5)
//		- Use specified power control or not(always on).
//		If you want to change UART baudrate then define "a3gsBAUDRATE"
//		symbol before #include "a3gs3.h" statement.
//***********************************************************

#ifndef _A3GIM_H_
#define _A3GIM_H_  

#include <Arduino.h>
#include <SoftwareSerial.h>

/*
	Define constants
*/
//	for compatibility of GSM.h
#define	ctrlz								26		// Ascii character for ctr+z. End of a SMS.
#define	cr									13		// Ascii character for carriage return. 
#define	lf									10		// Ascii character for line feed. 

//	Basic constants
#define	a3gsBAUDRATE						9600		// Default UART baudrate if "a3gsBAUDRATE" is undefined(3GIM default baudrate is 9600)
#define	a3gsDATE_SIZE						11		// date space size ("YYYY/MM/DD\0" format, used in getTime(), included '\0')
#define	a3gsTIME_SIZE						9			// time space size ("HH:MM:SS\0" format(hour is 24h-way), used in getTime(), included '\0')
#define	a3gsCS_ASCII						0			// SMS Text Char Code: ASCII (used in sendSMS())
#define	a3gsCS_UNICODE						1			// SMS Text Char Code: UNICODE(Little Endian) (used in sendSMS()) --@Not used R4.0
#define	a3gsIMEI_SIZE						16		// imei space size ("99..9\0" format, used in getIMEI(), included '\0')
#define	a3gsDEFAULT_PORT					0			// Default port number(for httpGET() and httpPOST())

//	Return values in general
#define	a3gsSUCCESS							0
#define	a3gsERROR								(-1)

//	Maximum lengths
#define	a3gsMAX_VERSION_LENGTH			5			// Maximum bytes of Version number(used in getVersion())
#define	a3gsMAX_SMS_LENGTH				120		// Maximum bytes of SMS message(used in sendSMS()) --@change R4.0
#define	a3gsMAX_MSN_LENGTH				11		// Maximum bytes of Phone Number in Japan(used in readSMS())
#define	a3gsMAX_HOST_LENGTH				96		// Maximum length of host name --@change R4.0
#define	a3gsMAX_DATA_LENGTH				1024		// Maximum length of data(at read/write)
#define	a3gsMAX_SMS_SLOT				20		// Maximum number of SMS slot --@add R4.0
#define	a3gsMAX_TUNNEL_DATA_LENGTH 32000	// Maximum length of data in writeBegin() --@add R4.0

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	// Mega and Mega 2560/ADK
#define	a3gsMAX_URL_LENGTH				256		// Maximum length of URL(used in httpGET() and httpPOST())
#define	a3gsMAX_RESULT_LENGTH			1024		// Maximum length of result(used in httpGET() and httpPOST())
#define	a3gsMAX_HEADER_LENGTH			512		// Maximum length of header(used in httpPOST())
#define	a3gsMAX_BODY_LENGTH				1024		// Maximum length of header(used in httpPOST())
#define	a3gsMAX_TWEET_LENGTH			140		// Maximum length of Tweet message(used in tweet())
#else // UNO, Lenoard, Pro, Fio..
#define	a3gsMAX_URL_LENGTH				128		// Maximum length of URL(used in httpGET() and httpPOST())
#define	a3gsMAX_RESULT_LENGTH			192		// Maximum length of result(used in httpGET() and httpPOST())
#define	a3gsMAX_HEADER_LENGTH			192		// Maximum length of header(used in httpPOST())
#define	a3gsMAX_BODY_LENGTH				256		// Maximum length of header(used in httpPOST())
#define	a3gsMAX_TWEET_LENGTH			60		// Maximum length of Tweet message(used in tweet())
#endif

//	Return values of getService()
#define	a3gsSRV_NO								0			// Out of service
#define	a3gsSRV_PS								1			// Data(packet) only
#define	a3gsSRV_CS								2			// Voice only -- @Not used R4.0
#define	a3gsSRV_BOTH							3			// Data and voice both -- @Not used R4.0

//	Method of positioning by getPosition() -- @Not used R4.0 
#define	a3gsMPBASED								0			// GPS + AGPS
#define	a3gsMPASSISTED						1			// AGPS
#define	a3gsMPSTANDALONE					2			// GPS only

/*
	Declare class
*/
class A3GS
{
  public:
  	A3GS() : _powerPin(0) { };
	enum A3GS_st_e { ERROR, IDLE, READY, TCPCONNECTEDCLIENT };

	// compatible methods with Arduino GSM/GPRS Shield library
	int getStatus() { return _status; };
	int begin(char* pin = 0);
	int begin(char* pin, uint32_t baudrate);
	int end(void);
	int restart(int pin = 0);   // @change(R3.0)
	int start(int pin = 0);      // @change(R3.0)
	int shutdown(void);
		//-- This library do not use "pin" parameter, so ignore it.
	int getIMEI(char* imei);
	int connectTCP(const char* server, int port);
	int disconnectTCP(void);
	int getStatusTCP(int *status, int *tcpnotif, long *remainedBytes, long *receivedBytes);	//@add R4.0
	int setTCPParams(int timeout1, int timeout2);	//@add R4.3 -- use only 3GIM(V2.2) above
	int write(uint8_t c);
			//--@R3.0 support binary data
	int write(const char* str);
			//--@R3.0 this function doesn't support binary data
	int write(const uint8_t* buffer, size_t sz);
			//--@R3.0 support binary data
	int writeBegin(size_t sz);		//@add R4.0
	int read(char* result, int resultlength);	//@R2.3 - Leave for compatibility
			//--@R3.0 change non-blocking mode and this function doesn't support binary data
	int read(void);		//@R2.0 Change
			//--@R3.0 support binary data and change non-blocking mode
	int read(uint8_t* buffer, size_t sz);		//@R2.3 Add
			//--@R3.0 support binary data and change non-blocking mode
		//-- This library do not support tcp/ip server functions.
	int httpGET(const char* server, uint16_t port, const char* path, char* result, int resultlength, boolean ssled = false, const char* header = NULL);
			//--@R3.0 add optional paramer "header"
	int httpPOST(const char* server, uint16_t port, const char* path, const char* header, const char* body, char* result, int* resultlength, boolean ssled = false);
			//-- httpPOST() is not compatible parameters with Arduino GSM/GPRS Shield library
	int tweet(const char* token, const char* msg);
		//-- tweet() sends a message to twitter with the text "msg"and the token "token". 
		//-- Get the token from http://arduino-tweet.appspot.com/

	//-- Extended methods of IEM 3G Shield --//
	int getLocation(int method, char* latitude, char* longitude);
	int getLocation2(char* latitude, char* longitude, char *height, char *utc, int *quality, int *number);	//@add R4.0
	int setLocationParams(int timeout, boolean useAGPS, boolean useActiveAntenna);	//@add R4.3 -- use only 3GIM(V2.2) above
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
	int enterAT(uint32_t duration);		//@add R4.0
	int getResult(char *buf, int *len, uint32_t timeout);
	
  private:
	int _status;		// for Compatible with GSM.h
	int _powerPin;
	void sendCommand(const char* cmd);
	void sendData(const char* data);
	void discardUntil(const char match);
};

extern	A3GS				a3gs;				// An A3GS object
extern  SoftwareSerial	a3gSerial;		// Serial for 3GIM

#endif // _A3GIM_H_
