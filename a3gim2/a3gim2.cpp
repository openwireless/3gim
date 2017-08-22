//***********************************************************
//	a3gim2.cpp -- Arduino Control Library for 3GIM(V2) for Mega/Leonardo/Zero/M0..
//
//	History:
//		R1.0 2012/10/06  1st Release for IEM 3G Shiled(Ver1.0)
//		R1.1 2012/10/14  2nd Release(Operate by few memories)
//						 (modify sendSMS(),onSMSReceived(),httpGET(),httpPOST(),tweet(),connectTCP())
//		R1.2 2012/10/28  Bug fix (httpGET(), httpPOST(), tweet(), availableSMS() and redSMS())
//		R1.3 2013/01/01  Support "https" in httpGET() and httpPost() [if gw3g version is above 1.1]
//		R1.4 2013/04/16  Bug fix (discardUntil() has never return)
//		R2.0 2013/07/12  Change interface of TCP/IP and discardUntil()
//						 some bugs fix(httpPOST, httpGET, read and getResult)
//		R2.1 2013/08/17  Buf fix (discardUntil() has never return)
//		R2.2 2013/10/27  Bug fix (read(res, reslength))
//		R2.3 2014/07/06  Support binary data in read() and add new version read() function, bug fix write()
//		R3.0 2014/08/30  Support for gw3g R2.0 and add follow functions:
//						   setAirplaneMode(), put(), get()
//		R3.0 2014/12/10  Support 3GIM, rename as a3gim.cpp
//		R3.1 2015/03/22  Tested to 3GIM(V1.1)
//		                           Support updateProfile() and encryptProfile()
//		R4.0 2016/02/27  Support 3GIM(V2) only
//		R4.1 2016/05/05  Support Large RAM size(4KB..) only
//		R4.2 2016/05/15  Bug fix enterAT()
//		R4.3 2017/08/15  Support 3GIM(V2.2): Remove SMS functions, add the following functions
//							setLocationParams(), setTCPParams()
//                       Bug fixed the following functions
//							setDefaultProfile(), enterAT()
//
//	Author:
//		Open wireless Alliance(Old 3G Shield Alliance) and Atushi Daikoku
//
//	Notes:
//		Lower compatible with Arduino GSM/GPRS Shield library.
//		Notices as bellow:
//		- Use HardwareSerial.
//		- Use specified power control pin or not(always on).
//		If You use Leonard, Mega or ADK(Mega2560) then you must change IEM_RXD_PIN and IEM_TXD_PIN for suitable.
//***********************************************************

// #define	DEBUG						// Define if you want to debug this library

#include "Arduino.h"
#include "a3gim2.h"

// Macros
#ifdef DEBUG
#  define debugSerial				Serial
#  define DEBUG_PRINT(m,v)			{ debugSerial.print("** "); debugSerial.print((m)); debugSerial.print(":"); debugSerial.println((v)); }
#  define DEBUGP(m)         		debugSerial.print((m))
#else
#  define DEBUG_PRINT(m,v)			// do nothing
#  define DEBUGP(m)         		// do nothing
#endif
	// Constants for pins
#define	IEM_RXD_PIN					4			// D4(For example, If you use Leonardo or Mega etc. then change to D10..)
#define	IEM_TXD_PIN					5			// D5(For example, If you use Leonardo or Mega etc. then change to D11..)
#define	IEM_POWER_PIN				6			// D6
	// Constants for getTime2()
#define	SECS_PER_MIN 				(60UL)
#define	SECS_PER_HOUR				(3600UL)
#define	SECS_PER_DAY				(SECS_PER_HOUR * 24UL)
#define	DAYS_PER_WEEK				(7UL)
#define	SECS_YR_2000				(946684800UL)		// the time at the start of y2k
	// Leap year calulator expects year argument as years offset from 1970
#define	LEAP_YEAR(Y)				(((1970+Y)>0) && !((1970+Y)%4) && (((1970+Y)%100) || !((1970+Y)%400)))
	// Command execution timeout values
#define	TIMEOUT_LOCAL				5000		// Timeout value of local functions [mS]
#define	TIMEOUT_NETWORK				65000		// Timeout value of communication functions [mS]
#define	TIMEOUT_GPS					185000		// Timeout value og GPS locationing [mS]
	// Misc.
#define	ISDIGIT(c)					((c) >= '0' && (c) <= '9')
	// IEM Version
#define	MIN_IEM_VERSION				3.3			// A necessary minimum firmware version of this library
	// Misc.
#define	MAX_RETRY					3			// Max retry count for begin()

// Define an A3GS(Arduino 3G Shield) Object Here.
A3GS	a3gs;

// Global variables
static char gWorkBuffer[256];			// Buffer for working(Mega..)


//***************************
//	begin()
//
//	@description
//		Begin to use 3G shield function
//	@return value
//		0 .. OK(at least, Packet Switch or Circuit Switch is availabled)
//		otherwise .. NG
//	@param
//		pin : Not used
//	@note
//		Change at R3.1 for 3GIM
//***************************
int A3GS::begin(char* pin)
{
	char	version[a3gsMAX_VERSION_LENGTH+1];
//--
	DEBUG_PRINT(">begin()", a3gsBAUDRATE);

	// Begin SoftwareSerial
	a3gSerial.begin(a3gsBAUDRATE);

	// Initilalize global variables
	_status = IDLE;

	// Get iem version and check it
	int n;
	for (n = 0; n < MAX_RETRY; n++) {
		if (getVersion(version) == 0)
			break;
		delay(10);
	}
	if (n == MAX_RETRY)
		return 1;	// NG -- Can't get version

	if (atof(version) < MIN_IEM_VERSION) {
		DEBUG_PRINT(">getVersion()", "IEM version is old");
		return 2;	// NG -- IEM Version is old
	}

	return a3gsSUCCESS;  // OK
}

//***************************
//	begin()
//
//	@description
//		Begin to use 3G shield function
//	@return value
//		0 .. OK(at least, Packet Switch or Circuit Switch is availabled)
//		otherwise .. NG
//	@param
//		pin : Not used
//		baudrate : baudrate to begin (2400/4800/9600/19200/38400/57600/115200)
//	@note
//		This function use to specify non-default baudrate.
//		Change at R3.1 for 3GIM
//***************************
int A3GS::begin(char* pin, uint32_t baudrate)
{
	char	version[a3gsMAX_VERSION_LENGTH+1];
//--
	DEBUG_PRINT(">begin()", baudrate);

	// Begin SoftwareSerial
	a3gSerial.end();
	a3gSerial.begin(baudrate);

	// Initilalize global variables
	_status = IDLE;

	// Get iem version and check it
	int n = 0;
	for (n = 0; n < MAX_RETRY; n++) {
		if (getVersion(version) == 0)
			break;
		delay(10);
	}
	if (n == MAX_RETRY)
		return 1;	// NG -- Can't get version

	if (atof(version) < MIN_IEM_VERSION) {
		DEBUG_PRINT(">getVersion()", "IEM version is old");
		return 2;	// NG -- IEM Version is old
	}

	return a3gsSUCCESS;  // OK
}

//***************************
//	end()
//
//	@description
//		End to use 3G shield function
//	@return value
//		0 .. always
//	@param
//		none
//	@note
//		re-enable serial communication, call a3gs.begin()
//		Change at R3.1 for 3GIM
//***************************
int A3GS::end(void)
{
	// End SoftwareSerial
	a3gSerial.flush();	//--@R3.0 add
	a3gSerial.end();

	return a3gsSUCCESS;	// OK
}

//***************************
//	restart()
//
//	@description
//		Restart 3G shield and clear SMS handler
//	@return value
//		0 .. always
//	@param
//		pin : Not used
//	@note
//		Reboot time is about 40 Sec.
//		Change at R3.1 for 3GIM
//***************************
int A3GS::restart(int pin)
{
	sendCommand("$YE");		// Send "Reset IEM" Command

	return a3gsSUCCESS;	// OK
}

//***************************
//	start()
//
//	@description
//		Power on 3G shield and wait for ready to use
//	@return value
//		0 .. always
//	@param
//		pin : 3GIM power control pin(if > 0) or not used(if <= 0)
//	@note
//		Start up time is about 15 Sec.
//		Change at R3.1 for 3GIM
//		Change at R4.0 for 3GIM(V2)
//***************************
int A3GS::start(int pin)
{
	if (pin > 0)
		_powerPin = pin;

	// Turn on 3GIM
	if (_powerPin > 0) {
		pinMode(_powerPin, OUTPUT);
		digitalWrite(_powerPin, HIGH);
		delay(200);
		digitalWrite(_powerPin, LOW);
	}

	a3gSerial.begin(9600);

	DEBUG_PRINT(">start()", "Turn on and wait for a moment..");

	delay(15000);	// Wait for ready IEM

	DEBUG_PRINT(">start()", "IEM is available now.");

	return a3gsSUCCESS;	// OK
}

//***************************
//	shutdown()
//
//	@description
//		Power off 3G shield
//	@return value
//		0 .. always
//	@param
//		none
//	@note
//		Shutdown time is about 15 Sec.
//***************************
int A3GS::shutdown(void)
{
	sendCommand("$YE");		// Send "Reset 3GIM" Command
	delay(2000);		// Wait a moment

	// Turn off 3GIM
	if (_powerPin > 0)
		digitalWrite(_powerPin, HIGH);

	return a3gsSUCCESS;	// OK
}

//***************************
//	getService
//
//	@description
//		Get network service status
//	@return value
//		0 .. Succeeded, otherwise .. Failed
//	@param
//		status .. [OUT] SRV* (see a3gs.h)
//	@note
//		
//***************************
int A3GS::getServices(int& status)
{
	char responses[12];		// Format "$YS=OK 9" or "$YS=NG errno"
	int  length = sizeof(responses);
//--
	sendCommand("$YS");	// Send "Get Services" command

	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getServices()", responses);

	// parse response
	switch (responses[7]) {
		case '0' :		// NO_SRV
			status = a3gsSRV_NO;
			break;
		case '1' :		// PS_ONLY
			status = a3gsSRV_PS;
			break;
		case '2' :		// CS_ONLY
			status = a3gsSRV_CS;
			break;
		case '3' :		// BOTH
			status = a3gsSRV_BOTH;
			break;
		default : 		// bug
			return 1;		// NG -- Can't get service
	}

	return a3gsSUCCESS;		// OK
}

//***************************
//	getIMEI()
//
//	@description
//		Get IEM's IMEI
//	@return value
//		0 .. Succeeded
//		otherwise .. Failed
//	@param
//		imei : [OUT] IMEI (a3gsIMEI_SIZE bytes space requred)
//	@note
//		
//***************************
int A3GS::getIMEI(char* imei)
{
	char responses[a3gsIMEI_SIZE+10];  // Format "$YI=OK 99..99" or "$YI=NG errno"
	int  length = sizeof(responses);
//--
	sendCommand("$YI");		// Send "Get IMEI" command

	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 1;  // NG -- maybe timeout

	DEBUG_PRINT(">getIMEI()", responses);

	// parse response
	if (strncmp(responses, "$YI=OK", 6))
		return 1;	// NG -- Can't get IMEI

	strncpy(imei, responses+7, a3gsIMEI_SIZE);
	imei[a3gsIMEI_SIZE] = '\0';

	return a3gsSUCCESS;	// OK
}

//***************************
//	setBaudrate
//
//	@description
//		Set UART baudrate
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		baudrate : baudrate to set (2400/4800/9600/19200/38400/57600/115200)
//	@note
//		New baudrate will be validated after reset IEM
//		Change at R4.0 for 3GIM(V2)
//***************************
int A3GS::setBaudrate(uint32_t baudrate)
{
	char command[13];
	char responses[12];  // Format "$YB=OK" or "$YB=NG errno"
	int  length = sizeof(responses);
//--
	switch (baudrate) {
//	case 2400 :
//	case 4800 :
		case 9600 :
		case 19200 :
		case 38400 :
		case 57600 :
		case 115200 :
			break;
		default :
			return 1;	// NG -- Bad parameter
	}

	// Make command and send it
	sprintf(command, "$YB %lu", baudrate);
	sendCommand(command);	// Send "Set Baudrate" command

	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 1;  // NG -- maybe timeout

	DEBUG_PRINT(">setBaudrate()", responses);

	// parse response
	if (strncmp(responses, "$YB=OK", 6))
		return 1;	// NG -- Can't set baudrate

	return a3gsSUCCESS;	// OK
}

//***************************
//	setLED
//
//	@description
//		Set LED On/Off
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		sw : true is On, false is Off
//	@note
//		There is LED on 3G shield board(named LED1)
//***************************
int A3GS::setLED(boolean sw)
{
	char command[10];
	char responses[12];  // Format "$YL=OK" or "$YL=NG errno"
	int  length = sizeof(responses);
//--
	// Make command and send it
	sprintf(command, "$YL %d", (sw ? 1 : 0));
	sendCommand(command);	// Send "Set LED" command

	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 1;  // NG -- maybe timeout

	DEBUG_PRINT(">setLED()", responses);

	// parse response
	if (strncmp(responses, "$YL=OK", 6))
		return 1;	// NG -- Can't set led

	return a3gsSUCCESS;	// OK
}

//***************************
//	setAirplaneMode
//
//	@description
//		Set airplane mode
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		sw : true is On(Airplane mode), false is Off(Normal mode)
//	@note
//		R3.0 add
//***************************
int A3GS::setAirplaneMode(boolean sw)
{
	char command[10];
	char responses[12];  // Format "$YP=OK" or "$YP=NG errno"
	int  length = sizeof(responses);
//--
	// Make command and send it
	sprintf(command, "$YP %d", (sw ? 1 : 0));
	sendCommand(command);	// Send "Set Airplane mode" command

	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 1;  // NG -- maybe timeout

	DEBUG_PRINT(">setAirplaneMode()", responses);

	// parse response
	if (strncmp(responses, "$YP=OK", 6))
		return 1;	// NG -- Can't set airplane mode

	return a3gsSUCCESS;	// OK
}

//***************************
//	httpGET
//
//	@description
//		Request HTTP/GET to server and port
//
//	@return value
//		0 .. OK
//		otherwise .. NG
//	@param
//		server : server name(ex. "www,google.com")
//		port : port number(ex. 80)
//		path : path(ex. service/index.html)
//		result : [OUT] responce raw data(no escaped, '\0' terminated)
//		resultlength ; "result" size(max MAX_RESULT_LENGTH)
//		ssled : if true then use https(SSL), otherwise use http
//	@note
//		This function is synchronized.
//		Support optional parameter "header" since @R3.0
//***************************
int A3GS::httpGET(const char* server, uint16_t port, const char* path, char* result, int resultlength, boolean ssled, const char* header)
{
	int	length = sizeof(gWorkBuffer);
	int	nbytes;
	int	c;		//@R2.0 Change
//--
	// Send command line little by little
	if (ssled) {
		if (strlen(server) + strlen(path) + 8 > a3gsMAX_URL_LENGTH)
			return 1;  // NG -- Too long url

		sendData("$WG https://");
		sendData(server);
		if (port != a3gsDEFAULT_PORT) {
			char ports[6];
			sendData(":");
			sprintf(ports, "%u", port);
			sendData(ports);
		}
	}
	else {
		if (strlen(server) + strlen(path) + 7 > a3gsMAX_URL_LENGTH)
			return 1;  // NG -- Too long url

		sendData("$WG http://");
		sendData(server);
		if (port != a3gsDEFAULT_PORT) {
			char ports[6];
			sendData(":");
			sprintf(ports, "%u", port);
			sendData(ports);
		}
	}

	sendData(path);	

	// Request with extra header --@R3.0 add
	if (header != NULL) {
		sendData(" \"");
		sendData(header);
		sendData("\"");
	}
	
	sendCommand("");	// Go HTTP/GET request

	// Discard gerbage in input buffer --@add R4.3
	while (a3gSerial.available() > 0)
		(void)a3gSerial.read();
	
	DEBUG_PRINT(">httpGET()", "REQ");

	if (getResult(gWorkBuffer, &length, TIMEOUT_NETWORK))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">httpGET()", gWorkBuffer);
		// result's format: $WG=OK nbytes "response"

	// Parse response
	if (strncmp(gWorkBuffer, "$WG=OK", 6))
		return 1;	// NG -- Can't get response

	nbytes = atoi(gWorkBuffer + 7);
	DEBUG_PRINT(">httpGET() nbytes", nbytes);

	// Copy response body into "result" and set "resultlength"
	uint32_t	startTime = millis();
	for (int n = 0; n < nbytes; n++) {
		while (a3gSerial.available() <= 0) {
			// Wait until valid response
			if (millis() - startTime >= TIMEOUT_NETWORK)
				return 2;	// NG -- Timeout
		}
		c = a3gSerial.read();
		if (n < resultlength - 1)
			result[n] = c;
	}
	discardUntil('\n');	// discard last '\n' -- @R2.0 Change
	if (nbytes > resultlength - 1)			//@R2.0 Change
		result[resultlength - 1] = '\0';
	else
		result[nbytes] = '\0';

	return a3gsSUCCESS;		// OK
}

//***************************
//	httpPOST
//
//	@discription
//		Request HTTP/POST to server and port
//
//	@return value
//		0 .. OK
//		otherwise .. NG
//  @param
//		server : server name(ex. "www,google.com")
//		port : port number(ex. 80)
//		path : path(ex. service/index.html)
//		header : header(without Content-Length, nul terminate, max MAX_HEADER_LENGTH)
//		body : request body(nul terminate, max MAX_BODY_LENGTH)
//		result : [OUT] responce raw data(no escaped)
//		resultlength ; [OUT] "result" size(max MAX_RESULT_LENGTH)
//		ssled : if true then use https(SSL), otherwise use http
//	@note
//		This function is synchronized.
//***************************
int A3GS::httpPOST(const char* server, uint16_t port, const char* path, const char *header, const char *body, char* result, int* resultlength, boolean ssled)
{
	int	length = sizeof(gWorkBuffer);
	int	nbytes;
	int	c;		//@R2.0 Change
//--
	if (strlen(body) > a3gsMAX_BODY_LENGTH)
		return 1;	// NG -- too long body
	if (header != NULL && strlen(header) > a3gsMAX_HEADER_LENGTH)
		return 1;	// NG -- too long header

	// Send command line little by little
	if (ssled) {
		if (strlen(server) + strlen(path) + 8 > a3gsMAX_URL_LENGTH)
			return 1;  // NG -- Too long url

		sendData("$WP https://");
		sendData(server);
		if (port != a3gsDEFAULT_PORT) {
			char ports[6];
			sendData(":");
			sprintf(ports, "%u", port);
			sendData(ports);
		}
	}
	else {
		if (strlen(server) + strlen(path) + 7 > a3gsMAX_URL_LENGTH)
			return 1;  // NG -- Too long url

		sendData("$WP http://");
		sendData(server);
		if (port != a3gsDEFAULT_PORT) {
			char ports[6];
			sendData(":");
			sprintf(ports, "%u", port);
			sendData(ports);
		}
	}

	if (path[0] != '/')
		sendData("/");		// for compatiblity old version
	sendData(path);

	sendData(" \"");

	sendData(body);

	if (header != NULL && *header != '\0') {
		sendData("\" \"");
		sendData(header);
	}

	sendCommand("\"");	// Go command

	// Discard gerbage in input buffer --@add R4.3
	while (a3gSerial.available() > 0)
		(void)a3gSerial.read();
	
	DEBUG_PRINT(">httpPOST()", "REQ");

	if (getResult(gWorkBuffer, &length, TIMEOUT_NETWORK))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">httpPOST()", gWorkBuffer);
		// result's format: $WP=OK nbytes or $WP=NG errno

	// Parse response
	if (strncmp(gWorkBuffer, "$WP=OK", 6))
		return 1;	// NG -- Can't post or get response

	nbytes = atoi(gWorkBuffer + 7);
	DEBUG_PRINT(">httpPOST() nbytes", nbytes);

	// Copy response body into "result" and set "resultlength"
	uint32_t	startTime = millis();
	for (int n = 0; n < nbytes; n++) {
		while (a3gSerial.available() <= 0) {
			// Wait until valid response
			if (millis() - startTime >= TIMEOUT_NETWORK)
				return 2;	// NG -- Timeout
		}
		c = a3gSerial.read();
		if (n < *resultlength - 1)
			result[n] = c;
	}
	discardUntil('\n');	// discard last '\n' -- @R2.0 Change
	if (nbytes > *resultlength - 1)			//@R2.0 Change
		result[*resultlength - 1] = '\0';
	else {
		result[nbytes] = '\0';
		*resultlength = nbytes;
	}

	return a3gsSUCCESS;		// OK
}

//***************************
//	tweet
//
//	@description
//		Tweet a message
//
//	@return value
//		0 .. OK
//		otherwise .. NG
//	@param
//		token .. token from http://arduino-tweet.appspot.com/
//		msg .. message to tweet(in UTF-8)
//	@note
//		This function use cloud service on "arduino-tweet.appspot.com".
//		Notice from this service:
//		- The library uses this site as a proxy server for OAuth stuff. Your tweet may not be applied during maintenance of this site.
//		- Please avoid sending more than 1 request per minute not to overload the server.
//		- Twitter seems to reject repeated tweets with the same contenet (returns error 403).
//		Use gWorkBuffer and destory it

//***************************
int A3GS::tweet(const char* token, const char* msg)
{
	int	resultlength = sizeof(gWorkBuffer);
//--
	if (strlen(msg) > a3gsMAX_TWEET_LENGTH)
		return 8;  // Too long message. (@@ check by number of characters, not bytes)

	sprintf(gWorkBuffer, "token=%s&status=%s", token, msg);

	// request POST to http://arduino-tweet.appspot.com
	return httpPOST("arduino-tweet.appspot.com", 80, "update", NULL, gWorkBuffer, gWorkBuffer, &resultlength);
}

//***************************
//	getTime
//
//	@description
//		Get current date and time
//	@return value
//		0 .. OK
//		otherwise .. NG
//	@param
//		date : [OUT] current date(JST) ("YYYY/MM/DD" format)
//		time : [OUT] current time(JST) ("HH:MM:SS" format)
//	@note
//		Change at R4.0 for 3GIM(V2)
//***************************
int A3GS::getTime(char date[], char time[])
{
	char	responses[28];  // Format "$YT=OK 2012/03/18 13:28:25"
	int	length = sizeof(responses);
//--
	sendCommand("$YT");		// Send "Get Time" command

	if (getResult(responses, &length, TIMEOUT_NETWORK))		//@change R4.0 
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getTime()", responses);

	if (strncmp(responses, "$YT=OK", 6))
		return 1;	// NG -- Can't get time

	// Parse response
	strncpy(date, responses + 7, 10);
	date[10] = '\0';

	strncpy(time, responses + 18, 8);
	time[8] = '\0';

	return a3gsSUCCESS;	// OK
}

//***************************
//	getTime2
//
//	@description
//		Get the current time as seconds since Jan 1 1970
//	@return value
//		0 .. OK
//		otherwise .. NG
// 	@param
//		seconds : [OUT] Current seconds since Jan 1 1970(JST)
//	@note
//		
//***************************
int A3GS::getTime2(uint32_t& seconds)
{
	static  const uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	char date[a3gsDATE_SIZE];
	char time[a3gsTIME_SIZE];
	uint8_t year, month, day, hour, minute, second;
	uint32_t t;   // total seconds from 1970/01/01 00:00:00
//--
	// Get Current Time
	if (getTime(date, time) != 0)
		return 1;	// NG -- can't get time, Failed.

	year = atoi(date) - 1970;
	month = atoi(date+5);
	day = atoi(date+8);
	hour = atoi(time);
	minute = atoi(time+3);
	second = atoi(time+6);

	// Seconds from 1970 till 1 jan 00:00:00 of the given year
	t = year * (SECS_PER_DAY * 365);
	for (int y = 0; y < year; y++) {
		if (LEAP_YEAR(y))
			t += SECS_PER_DAY;	// add extra days for leap years
	}

	// Add days for this year, months start from 1
	for (int m = 1; m < month; m++) {
		if ((m == 2) && LEAP_YEAR(year))
			t += SECS_PER_DAY * 29;
		else
			t += SECS_PER_DAY * monthDays[m-1];  //monthDay array starts from 0
	}
	t += (day - 1) * SECS_PER_DAY;
	t += hour * SECS_PER_HOUR;
	t += minute * SECS_PER_MIN;
	t += second;

	seconds = t;

	return a3gsSUCCESS;	// OK
}

//***************************
//	getRSSI
//
//	@description
//		Get 3G RSSI
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		rssi : [OUT] rssi (dBm)
//	@note
//		
//***************************
int A3GS::getRSSI(int& rssi)
{
	char	responses[13];  // Format "$YR=OK -999"
	int	length = sizeof(responses);
//--
	sendCommand("$YR");	// Send "Get RSSI" command

	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getRSSI()", responses);

	// Check response is ok --@add R4.3
	if (responses[4] != 'O' || responses[5] != 'K')
		return 1;	// NG -- can't get rssi

	// Parse response
	rssi = atoi(responses + 7);

	return a3gsSUCCESS;	// OK
}

//***************************
//	getVersion
//
//	@description
//		Get Version
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		version : [OUT] version ("X.YY")
//	@note
//		
//***************************
int A3GS::getVersion(char *version)
{
	char	responses[13];  // Format "$YV=OK 9.99"
	int	length = sizeof(responses);
//--
	sendCommand("$YV");	// Send "Get Version" command

	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getVersion()", responses);

	// Copy version
	if (strncmp(responses, "$YV", 3) != 0)
		return 2;	// NG -- can't get version

	strcpy(version, responses + 7);

	return a3gsSUCCESS;	// OK
}

//***************************
//	getLocation
//
//	@description
//		get Current Location from GPS and Network
//
//	@return value
//		0 .. OK
//		1 .. NG(Bad parameters)
//		otherwise .. NG(Can't locate position..)
//	@param
//		method .. method of locate(a3gsM)
//		latitude .. [OUT] latitude
//		longitude .. [OUT] longitude
//	@note
//		10-180 Sec. required.
//***************************
int A3GS::getLocation(int method, char* latitude, char* longitude)
{
	char *cp, responses[40];	// Format "$LG=OK lat lng"
	int	length = sizeof(responses);
//--
	*latitude = '\0';
	*longitude = '\0';

	// Make command and send it
	switch (method) {
		case a3gsMPBASED :
			sendCommand("$LG MSBASED");  // Default is method=MSBASED
			break;
		case a3gsMPASSISTED :
			sendCommand("$LG MSASSISTED");
			break;
		case a3gsMPSTANDALONE :
			sendCommand("$LG STANDALONE");
			break;
		default :
			return 1;  // bad method
	}

	if (getResult(responses, &length, TIMEOUT_GPS))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getLocation()", responses);

	// parse response
	if (! strncmp(responses, "$LG=NG", 6))
		return 1;	// NG --  Can't locate

	// Parse lattitude (assume enough space to store latitude)
	for (cp = responses+7; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp) || *cp == '.')
			*latitude++ = *cp++;
		*latitude = '\0';
		DEBUG_PRINT(">getLocation() lat", latitude);
		break;
	}

	// Skip spaces
	while (*cp != '\0' && ! ISDIGIT(*cp))
		cp++;

	// Parse longitude (assume enough space to store longitude)
	for ( ; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp) || *cp == '.')
			*longitude++ = *cp++;
		*longitude = '\0';
		DEBUG_PRINT(">getLocation() lng", longitude);
		break;
	}

	return a3gsSUCCESS;	// OK
}

//***************************
//	getLocation2
//
//	@description
//		get Current Location from GPS and Network
//
//	@return value
//		0 .. OK
//		1 .. NG(Bad parameters)
//		otherwise .. NG(Can't locate position..)
//	@param
//		latitude .. [OUT] latitude
//		longitude .. [OUT] longitude
//		height .. [OUT]
//		utc .. [OUT]
//		quality .. [OUT] 
//		number .. [OUT]
//	@note
//		Add this function from R4.0
//***************************
int A3GS::getLocation2(char* latitude, char* longitude, char *height, char *utc, int *quality, int *number)
{
	int	length = sizeof(gWorkBuffer);
	char *cp;
//--
	*latitude = '\0';
	*longitude = '\0';

	sendCommand("$LG - 1");

	if (getResult(gWorkBuffer, &length, TIMEOUT_GPS))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getLocation2()", gWorkBuffer);

	if (! strncmp(gWorkBuffer, "$LG=NG", 6))
		return 1;	// NG --  Can't locate

	// Parse lattitude
	for (cp = gWorkBuffer+7; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp) || *cp == '.')
			*latitude++ = *cp++;
		*latitude = '\0';
		DEBUG_PRINT(">getLocation2() lat", latitude);
		break;
	}

	// Skip spaces
	cp++;

	// Parse longitude
	for ( ; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp) || *cp == '.')
			*longitude++ = *cp++;
		*longitude = '\0';
		DEBUG_PRINT(">getLocation2() lng", longitude);
		break;
	}
	
	// Skip spaces
	cp++;

	// Parse UTC
	for ( ; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp))
			*utc++ = *cp++;
		*utc = '\0';
		DEBUG_PRINT(">getLocation2() utc", utc);
		break;
	}

	// Skip ".mmm" and space
	cp += 5;

	// Parse quality
	*quality = atoi(cp);
	DEBUG_PRINT(">getLocation2() qty", *quality);

	// Skip quality and space(s)
	for ( ; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp))
			cp++;
		break;
	}
	cp++;	// skip space
	
	// Parse number
	*number = atoi(cp);
	DEBUG_PRINT(">getLocation2() num", *number);

	// Skip number and space
	for ( ; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp))
			cp++;
		break;
	}
	cp++;
	
	// Parse height
	for ( ; *cp != '\0'; cp++) {
		while (ISDIGIT(*cp) || *cp == '.' || *cp == '-')
			*height++ = *cp++;
		*height = '\0';
		DEBUG_PRINT(">getLocation2() hgt", height);
		break;
	}
	
	return a3gsSUCCESS;	// OK
}

//***************************
//	setLocationParams
//
//	@description
//		Set parameters for location function
//	@return value
//		0 .. OK
//		1 .. NG(Bad parameters)
//		2 .. NG(Internal error)
//	@param
//		timeout : Time to wait until the positioning is over [10mS], If the value is 0, do not change
//		useAGPS : Use(true) or not(false) AGPS function
//		useActiveAntenna : Use(true) or not(false) active GPS antenna
//	@note
//		Add this function from R4.3
//***************************
int A3GS::setLocationParams(int timeout, boolean useAGPS, boolean useActiveAntenna)
{
	if (timeout > 0) {
		char command[20], responses[20];  // Format "$LX=OK" or "$LX=NG errno"
		int  length = sizeof(responses);

		// Make command and send it
		sprintf(command, "$LX %d", timeout);
		sendCommand(command);	// Send "Set location parameters" command

		// Get response
		if (getResult(responses, &length, TIMEOUT_LOCAL))
			return 2;  			// NG -- maybe internal error

		// parse response
		if (strncmp(responses, "$LX=OK", 6))
			return 1;			// NG -- maybe bad params
	}

	if (useAGPS) {
		// Make AGPS function available
		char apn[30], user[20], password[20];
		if (getDefaultProfile(apn, user, password) == 0) {
			char atwppp[60];
			sprintf(atwppp, "at+wppp=2,4,\"%s\",\"%s\"", user, password);
			enterAT(2);
			a3gSerial.println(atwppp);
			delay(200);
		}
		else {
			return 2;	// NG - can't get default APN
		}
	}
	
	enterAT(1);
	if (useActiveAntenna) a3gSerial.println("AT+GPSCONF=1,1");
	else a3gSerial.println("AT+GPSCONF=1,3");
	// The second argument should be adjusted depending on the situation
	delay(100);

	return a3gsSUCCESS;	// OK
}

//***************************
//	setDefaultProfile
//
//	@description
//		Set default profile(APN) number
//	@return value
//		0 .. OK
//		1 .. NG(Bad parameters)
//		2 .. NG(Internal error - timeout)
//		3 .. NG(Internal error - $PS error)
//	@param
//		apn : APN
//		user : Auth user name
//		password : Auth password
//	@note
//		Change interface from R4.0
//***************************
int A3GS::setDefaultProfile(const char *apn, const char *user, const char *password)
{
	char responses[20];  // Format "$PS=OK\n" or "$PS=NG 999\n"
	int	length = sizeof(responses);
//--
	if (strlen(apn) + strlen(user) + strlen(password) > 51)
		return 1;	// NG -- Bad parameter(too long)

	sprintf(gWorkBuffer, "$PS \"%s\" \"%s\" \"%s\"", apn, user, password);
	sendCommand(gWorkBuffer);		// Send "Set Profile" command

	if (getResult(responses, &length, TIMEOUT_NETWORK))
		return 2;	// NG -- maybe timeout

	DEBUG_PRINT(">setDefaultProfile()", responses);

	// parse response
	if (! strncmp(responses, "$PS=NG", 6))
		return 3;	// NG --  Can't set prfoile number

	return a3gsSUCCESS;	// OK
}

//***************************
//	getDefaultProfile
//
//	@description
//		Get default profile(APN) number
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		apn : [OUT] APN
//		user : [OUT] Auth user name
//		password : [OUT] Auth password
//	@note
//		Change interface from R4.0
//***************************
int A3GS::getDefaultProfile(char *apn, char *user, char *password)
{
	int	length = sizeof(gWorkBuffer);
	char *p;
//--
	sendCommand("$PS");	// Send "Get Profile" command

	if (getResult(gWorkBuffer, &length, TIMEOUT_LOCAL))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getDefaultProfile()", gWorkBuffer);

	// parse response and set out parameters
	for (p = gWorkBuffer + 8; *p != '"'; p++)
		*apn++ = *p;
	*apn = '\0';
	for (p += 3; *p != '"'; p++)
		*user++ = *p;
	*user = '\0';
	for (p += 3; *p != '"'; p++)
		*password++ = *p;
	*password = '\0';
	
	return a3gsSUCCESS;  // Succeeded
}

//***************************
//	connectTCP
//
//	@description
//		Connect to server with TCP/IP connection
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		server : server name or IP address(ex. "www,google.com", "192.161.10.1")
//		port : port number(ex. 80)
//	@note
//		
//***************************
int A3GS::connectTCP(const char* server, int port)
{
	char	responses[30];		// FORMAT: "$TC=OK\n" or "$TC=NG errno [info]\n"
	int	length = sizeof(responses);
//--
	// Check parameter size
	if (strlen(server) > a3gsMAX_HOST_LENGTH)
		return 1;	// NG -- too long host name
	// Make command and send it
	sprintf(gWorkBuffer, "$TC %s %d", server, port);
	sendCommand(gWorkBuffer);

	if (getResult(responses, &length, TIMEOUT_NETWORK))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">connectTCP()", responses);

	// parse response
	if (! strncmp(responses, "$TC=NG", 6))
		return 1;	// NG --  Can't connect

	// Change status
	_status = TCPCONNECTEDCLIENT;

	return a3gsSUCCESS;	// OK
}

//***************************
//	disconnectTCP
//
//	@description
//		Disconnect from server with current TCP/IP connection
//	@return value
//		0 .. OK
//		1 .. NG
//	@param
//		none
//	@note
//		
//***************************
int A3GS::disconnectTCP()
{
	char	responses[30];		// FORMAT: "$TD=OK\n" or "$TD=NG errno [info]\n"
	int	length = sizeof(responses);
//--
	// Send command
	sendCommand("$TD");

	if (getResult(responses, &length, TIMEOUT_NETWORK))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">disconnectTCP()", responses);

	// parse response
	if (! strncmp(responses, "$TD=NG", 6))
		return 1;	// NG --  Can't connect

	// Change status
	_status = READY;

	return a3gsSUCCESS;	// OK
}

//***************************
//	getStatusTCP
//
//	@description
//		Get TCP/IP status
//	@return value
//		0 .. OK
//		1 .. NG (Internal error, etc.)
//	@param
//		status : [out] See this library user manual.
//		tcpnotif : [out] See this library user manual.
//		remainedBytes : [out] Number of bytes waiting to be sent
//		receivedBytes :  [out] Number of received bytes
//	@note
//		Add ths function from R4.3
//***************************
int A3GS::getStatusTCP(int *status, int *tcpnotif, long *remainedBytes, long *receivedBytes)
{
	int	length = sizeof(gWorkBuffer);
	char *p;
	// $TS=OK status tcpnotif remainedBytes receivedBytes\n
//--
	// Send command
	sendCommand("$TS1");

	if (getResult(gWorkBuffer, &length, TIMEOUT_LOCAL))
		return 1;	// NG -- maybe timeout

	DEBUG_PRINT(">getStatusTCP()", gWorkBuffer);
	
	// parse response and set out parameters
	for (p = gWorkBuffer + 7; isspace(*p); p++)
		;
	*status = atoi(p);
	while (ISDIGIT(*p))
		p++;	// skip number(status)
	while (isspace(*p))
		p++;	// skip space(s)
	*tcpnotif = atoi(p);
	while (ISDIGIT(*p))
		p++;	// skip number(status)
	while (isspace(*p))
		p++;	// skip space(s)
	*remainedBytes = atol(p);
	while (ISDIGIT(*p))
		p++;	// skip number(status)
	while (isspace(*p))
		p++;	// skip space(s)
	*receivedBytes = atol(p);

	return a3gsSUCCESS;	// OK
}

//***************************
//	setTCPParams
//
//	@description
//		Set TCP/IP parameters
//	@return value
//		0 .. OK
//		1 .. NG (Bad parameter)
//		2 .. NG (Internal error, etc.)
//	@param
//		timeout1 : timeout to connectTCP() [10mS]
//		timeout2 : timeout to read() and write() [10mS]
//	@note
//		Add ths function from R4.3
//***************************
int A3GS::setTCPParams(int timeout1, int timeout2)
{
	char command[30], responses[20];  // Format "$TX=OK" or "$TX=NG errno"
	int  length = sizeof(responses);

	// Check parameters
	if (timeout1 < 0 || timeout1 > 6000)
		return 1;			// NG -- Bad params
	if (timeout2 < 0 || timeout2 > 6000)
		return 1;			// NG -- Bad params
		
	// Make command and send it
	sprintf(command, "$TX %d %d", timeout1, timeout2);
	sendCommand(command);	// Send "Set TCP/IP parameters" command

	// Get response
	if (getResult(responses, &length, TIMEOUT_LOCAL))
		return 2;  			// NG -- maybe internal error

	// parse response
	if (strncmp(responses, "$TX=OK", 6))
		return 1;			// NG(Bad params, etc.)

	return a3gsSUCCESS;	// OK
}

//***************************
//	write
//
//	@description
//		Write byte into current TCP/IP connection
//	@return value
//		-1 .. NG (Error)
//		 1 .. OK
//	@param
//		c : byte data to write
//	@note
//		R2.3 bug fix(binary data escaping)
//***************************
int A3GS::write(uint8_t c)
{
	char	responses[30];		// FORMAT: "$TW=OK nbytes\n" or "$TW=NG errno [info]\n"
	int	length = sizeof(responses);
//--
	// Make command and send it
	sendData("$TW \"");
	if (c < 0x20) {
		char	escaped[6];
		sprintf(escaped, "$x%02x", c);
		a3gSerial.print(escaped); 
	}
	// @R2.3 add -begin-
	else if (c == '"') {
		a3gSerial.print("$\"");
	}
	else if (c == '$') {
		a3gSerial.print("$$");
	}
	// @R2.3 add -end-
	else {
		a3gSerial.print((char)c); 
	}
	sendCommand("\"");

	if (getResult(responses, &length, TIMEOUT_NETWORK))
		return -1;	// NG -- maybe timeout

	DEBUG_PRINT(">write()", responses);

	// parse response
	if (! strncmp(responses, "$TW=NG", 6))
		return -1;	// NG --  Can't connect

	int	nbytes = atoi(responses + 7);

	return nbytes;	// OK
}

//***************************
//	write
//
//	@description
//		Write string into current TCP/IP connection
//	@return value
//		-1 .. NG (Error)
//		0 < .. OK (= wrote bytes)
//	@param
//		str : string(= '\0' terminated byte array) to write
//	@note
//		"str" must be encoded by $ escape sequence
//***************************
int A3GS::write(const char* str)
{
	char	responses[30];		// FORMAT: "$TW=OK nbytes" or "$TW=NG errno [info]\n"
	int	length = sizeof(responses);
//--
	if (strlen(str) > a3gsMAX_DATA_LENGTH)
		return -1;	// NG -- too large data

	// Make command and send it
	sendData("$TW \"");
	sendData(str);
	sendCommand("\"");

	if (getResult(responses, &length, TIMEOUT_NETWORK))
		return -1;	// NG -- maybe timeout

	DEBUG_PRINT(">write()", responses);

	// parse response
	if (! strncmp(responses, "$TW=NG", 6))
		return -1;	// NG --  Can't connect

	int	nbytes = atoi(responses + 7);

	return nbytes;	// OK
}

//***************************
//	write
//
//	@description
//		Write byte data into current TCP/IP connection
//	@return value
//		-1 .. NG (Error)
//		0 < .. OK (= wrote bytes)
//	@param
//		buffer : data to write(byte array)
//		sz : data size(in byte)
//	@note
//		R2.3 bug fix(binary data escaping)
//***************************
int A3GS::write(const uint8_t* buffer, size_t sz)
{
	char	responses[30];		// FORMAT: "$TW=OK nbytes\n" or "$TW=NG errno [info]\n"
	int	length = sizeof(responses);
//--
	if (sz > (size_t)a3gsMAX_DATA_LENGTH)
		return -1;	// NG -- too large data

	// Make command and send it
	sendData("$TW \"");
	while (sz-- > 0) {
		if (*buffer < 0x20) {
			char	escaped[6];
			sprintf(escaped, "$x%02x", *buffer);
			a3gSerial.print(escaped); 
		}
		// @R2.3 add -begin-
		else if (*buffer == '"') {
			a3gSerial.print("$\"");
		}
		else if (*buffer == '$') {
			a3gSerial.print("$$");
		}
		// @R2.3 add -end-
		else {
			a3gSerial.print((char)*buffer); 
		}
		buffer++;
	}
	sendCommand("\"");

	if (getResult(responses, &length, TIMEOUT_NETWORK))
		return -1;	// NG -- maybe timeout

	DEBUG_PRINT(">write()", responses);

	// parse response
	if (! strncmp(responses, "$TW=NG", 6))
		return -1;	// NG --  Can't connect

	int	nbytes = atoi(responses + 7);

	return nbytes;	// OK
}

//***************************
//	writeBegin
//
//	@description
//		Begin to write byte stream into current TCP/IP connection
//		User will write binary/text data a3gSerial directly
//	@return value
//		-1 .. NG (Error)
//		0 < .. OK (= wrote bytes)
//	@param
//		sz : data size(in byte)
//	@note
//		Add R4.0 for 3GIM(V2)
//***************************
int A3GS::writeBegin(size_t sz)
{
	int	length = sizeof(gWorkBuffer);
//--
	if (sz > a3gsMAX_TUNNEL_DATA_LENGTH)
		return -1;	// NG -- Bad parameter

	// Make command and send it
	sprintf(gWorkBuffer, "$TT %ud", sz);
	sendCommand(gWorkBuffer);

	if (getResult(gWorkBuffer, &length, TIMEOUT_LOCAL))
		return (-1);		// NG -- can't read

	DEBUG_PRINT(">writeBegin()", gWorkBuffer);

	return a3gsSUCCESS;		// OK
}

//***************************
//	read
//
//	@description
//		Read data from current TCP/IP connection
//	@return value
//		-2  .. NG (Closed connection by other side)
//		-1  .. NG (Error)
//		 0  .. OK (No data)
//		0 < .. OK (= read bytes)
//	@param
//		result : [OUT] read data(byte array, not '\0' terminated)
//		resultlength : data length(in byte)
//	@note
//		R2.3 fix return value bug
//		This function leaves for compatibility
//***************************
int A3GS::read(char* result, int resultlength)
{
	int	length = sizeof(gWorkBuffer);
	int	c;		//@R2.0 Change
//--
	if (resultlength > a3gsMAX_DATA_LENGTH)
		return -1;	// NG -- Bad parameter

	// Make command and send it
	sprintf(gWorkBuffer, "$TR %d", resultlength);
	sendCommand(gWorkBuffer);

	if (getResult(gWorkBuffer, &length, TIMEOUT_NETWORK))
		return (-1);		// NG -- can't read

	DEBUG_PRINT(">read()", gWorkBuffer);

	// Parse response
	if (! strncmp(gWorkBuffer, "$TR=NG", 6)) {
		int errno = atoi(gWorkBuffer + 7);
		switch (errno) {
		  case 635 :		// Connection closed
		  case 636 :
			return (-2);
		  default :			// Other error
			return (-1);
		}
	}

	int nbytes = atoi(gWorkBuffer + 7);
	DEBUG_PRINT(">read() nbytes", nbytes);

	// Copy response body into "result" and set "resultlength"
	for (int n = 0; n < nbytes; ) {
		while ((c = a3gSerial.read()) < 0)
			;	// read until valid response
		if (n < resultlength - 1)
			result[n] = c;
		n++;
	}
	discardUntil('\n');		// discard last '\n' -- @R2.0 Change

	if (nbytes > resultlength - 1) {		//@R2.0 Change
		result[resultlength - 1] = '\0';
		return (resultlength - 1);	// OK
	}
	else {
		result[nbytes] = '\0';
		return nbytes;				// OK
	}
}

//***************************
//	read
//
//	@description
//		Read (binary) data from current TCP/IP connection
//	@return value
//		-2  .. NG (Closed connection by other side)
//		-1  .. NG (Error)
//		 0  .. OK (No data)
//		0 < .. OK (= read bytes)
//	@param
//		buffer : [OUT] read data(byte array, not '\0' terminated)
//		sz     : binary data size(in byte)
//	@note
//		R2.3 Add this function
//***************************
int A3GS::read(uint8_t* buffer, size_t sz)
{
	int	length = sizeof(gWorkBuffer);
	int	c;
//--
	if (sz > (size_t)a3gsMAX_DATA_LENGTH)
		return -1;	// NG -- Bad parameter

	// Make command and send it
	sprintf(gWorkBuffer, "$TR %d", sz);
	sendCommand(gWorkBuffer);

	if (getResult(gWorkBuffer, &length, TIMEOUT_NETWORK))
		return (-1);		// NG -- can't read

	DEBUG_PRINT(">read()", gWorkBuffer);

	// Parse response
	if (! strncmp(gWorkBuffer, "$TR=NG", 6)) {
		int errno = atoi(gWorkBuffer + 7);
		switch (errno) {
		  case 635 :		// Connection closed
		  case 636 :
			return (-2);
		  default :			// Other error
			return (-1);
		}
	}

	size_t nbytes = atoi(gWorkBuffer + 7);
	DEBUG_PRINT(">read() nbytes", nbytes);

	// Copy response body into "result" and set "resultlength"
	size_t n;
	for (n = 0; n < nbytes; n++) {
		while ((c = a3gSerial.read()) < 0)
			;	// read until valid response
		if (n < sz)
			buffer[n] = (uint8_t)c;
	}

	discardUntil('\n');		// discard last '\n'

	return (int)((n < sz) ? n : sz);		// OK
}

//***************************
//	read
//
//	@description
//		Read byte from current TCP/IP connection
//	@return value
//		-2  .. NG (Closed connection by other side)
//		-1  .. NG (Error)
//		-3  .. OK (No data)
//		 0..0xFF .. OK (= read byte)
//	@param
//		none
//	@note
//		This function is synchronized operation.
//		Change return value type at @R2.0
//***************************
int A3GS::read(void)
{
	int	length = sizeof(gWorkBuffer);
	int	c;		//@R2.0 Change
//--
	// Send command
	sendCommand("$TR 1");

	if (getResult(gWorkBuffer, &length, TIMEOUT_NETWORK))
		return (-1);	// NG -- can't read

	DEBUG_PRINT(">read()", gWorkBuffer);

	// Parse response
	if (! strncmp(gWorkBuffer, "$TR=NG", 6)) {
		int errno = atoi(gWorkBuffer + 7);
		switch (errno) {
		  case 635 :		// Connection closed
		  case 636 :
			return (-2);
		  default :			// Other error
			return (-1);
		}
	}

	size_t nbytes = atoi(gWorkBuffer + 7);
	DEBUG_PRINT(">read() nbytes", nbytes);

	if (nbytes == 1) {
		while ((c = a3gSerial.read()) < 0)
			;	// read valid response
	}
	else // if (nbytes == 0)
		c = -3;		// NG -- no data

	discardUntil('\n');	// discard last '\n' -- @R2.0 Change

	return c;	// OK
}

//***************************
//	enterAT
//
//	@description
//		Enter AT command pass-throuh mode only specified time
//	@return value
//		0  .. OK
//		1  .. NG (Parameter error)
//	@param
//		duration : The time that You can take advantage of [x100 mS]
//	@note
//		AT commands and responses are comminicated via a3gSerial directly.
//		Add @R4.0
//***************************
int A3GS::enterAT(uint32_t duration)
{
	int	length = sizeof(gWorkBuffer);
//--
	if (duration == 0)
		strcpy(gWorkBuffer, "$YA");
	else
		sprintf(gWorkBuffer, "$YA %u", (uint16_t)duration);
	sendCommand(gWorkBuffer);		// Send "Enter AT command" command

	if (getResult(gWorkBuffer, &length, TIMEOUT_LOCAL))
		return 1;	// NG -- maybe timeout

	delay(10);

	DEBUG_PRINT(">AT PASS THROUGH", duration);

	return a3gsSUCCESS;	// OK
}

//***************************
//	getResult
//
//	@description
//		Get response from 3GIM(V2)
//	@return value
//		0  .. OK
//		1  .. NG (timeout)
//	@param
//		buf : buffer to store result
//		len : [IN/OUT] buffer size and response length(in bytes)
//		timeout : [mS]
//	@note
//		Add @R4.0(moved from private to public)
//***************************
int A3GS::getResult(char *buf, int *len, uint32_t timeout)
{
	DEBUGP("getResult()");
	uint32_t ts = millis();
	boolean completed = false;
	int length = 0;
	while (! completed) {
		if (millis() - ts >= timeout) {
			DEBUG_PRINT(">getResult()", "TIMEOUT");
			return 1;		// NG -- Timeout
		}
		if (a3gSerial.available() <= 0)
			continue;
			DEBUGP(".");
		int c = a3gSerial.read();		//@R2.0 Change
		DEBUGP(((char)c));
		if (c == 0x0a) {	// end of line
			if (buf[0] == '$')
				completed = true;	// got result
			else
				length = 0;			// got status, so retry
		}
		else if (length < *len - 1)
			buf[length++] = c;
	}
	DEBUGP("");
	buf[length] = '\0';
	*len = length;

	DEBUG_PRINT("<getResult", buf);

	return 0;	// OK
}

//***
//  Private methods
//***

//  sendCommand() -- send command to iem with \n
void A3GS::sendCommand(const char* cmd)
{
	// Send command to IEM with '\n'
	a3gSerial.println(cmd); 
	DEBUG_PRINT("<sendCommand()", cmd);
}

//  sendData() -- send data to iem without \n
void A3GS::sendData(const char* data)
{
	// Send data to IEM without '\n'
	a3gSerial.print(data); 
	DEBUG_PRINT("<sendData()", data);
}

// discardUntl() -- discard characters from a3gSerial until match specified character
void A3GS::discardUntil(const char match)
{
	int	c;		//@R2.0 Change
	uint32_t ts = millis();
	boolean	done = false;
//--
	while (! done) {
		if (millis() - ts >= TIMEOUT_LOCAL) {
			DEBUG_PRINT(">discardUntil()", "TIMEOUT");
			break;		// Timeout !
		}

		if ((c = a3gSerial.read()) < 0)
			continue;	// discard until valid response -- @R2.1 Change

		if (c == match)
			done = true;
	}
}
	
// END OF a3gim2.cpp

