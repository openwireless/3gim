/*
 *  gw_ltem.h - Command interpreter for mgim
 *
 *  R0  2020.03.10 605e
 */

#ifndef _gw_ltem_h_
#define _gw_ltem_h_

#include <mgim.h>
#include <hl7800.h>

//#define DEBUG_USB

//
#define GW_LTEM_VERSION         "0.1"

// Define Symbols
#define	MAX_COMMAND_LENGTH		8192		// Actual maximum command length, include '\n' (in byets)
#define	MAX_RESULT_LENGTH		8192		// Actual maximum command result length, include '\n' (in byets)
#define	MAX_RESPONSE_LENGTH		4096		// Maximum http/get or http/post response length (in byets) - compatible with v1
#define	MAX_HOST_LENGTH			96			// Maximum length of host name with domain
#define MAX_IP_ADDRESS_LENGTH   15          // Maxmum IP(v4) address length (in bytes)
#define	MAX_PARAMS				4			// Maximum number of $command parameters
#define	MAX_SMS_MESSAGE_LENGTH	120			// Maximum length of SMS message (in bytes)
#define MAX_SMS_MESSAGES		20			// Maximum number of SMS (stored in HL8548-G)
#define	MAX_PORT_NUMBER			65535		// Maximum port number
#define	MAX_PROFILE_LENGTH		59			// Maximum profile length(apn + user + password, include " and ,) (in bytes)
#define	MAX_TCPREAD_SIZE		1024		// Maximum length of TCP read at one time (in bytes)
#define	MAX_TCPWRITE_SIZE		1024		// Maximum length of TCP write at one time (in bytes)
#define	MAX_TCPTUNNEL_SIZE		32000		// Maximum length of TCP write tunnel at one time (in bytes) -- limitation of HL8548-G
#define	MAX_UDPTUNNEL_SIZE		1020		// Maximum length of UDP send tunnel at one time (in bytes)
#define	MAX_IP_ADDRESS_LENGTH	15			// Maximum digits of ip address(v4) (ex. "192.168.100.12")
#define	MAX_GPS_REPEAT_COUNT	100			// Maximum GPS Repeat Counts
#define DATETIME_DIGITS         19
#define IMEI_DIGITS             15
// End of data string
#define	END_PATTERN				"@EOF@"		// Use for KHTTP*GET/KHTTP*POST
// Common timeout value
#define	TIMEOUT_LOCAL			200			// Timeout of local AT command [x10 mS]
#define	TIMEOUT_CGATT			6000		// Timeout of "AT+CGATT" [x10 mS]
#define	WAITTIME_LOCAL			30			// Wait time until execution local AT command [x10 mS]

// return values from command handler, doCommand(), getCommand(), getXxx(), parseXxx()..
#define	SUCCESS					          0
#define	ERROR					          1
#define	TIMEOUT					          (-1)
#define	BAD_COMMAND				          2
#define	TOO_LONG_COMMAND		          3
#define	BAD_FORMAT				          4
#define CANNOT_EXECUTE_IN_AIRPLANEMODE    5

 //
#define WELCOME_MESSAGE                   "\nWelcome to MGIM(v2)\n"

// Debug macros
#ifdef DEBUG_USB
# define DBG(...)       { mgSERIAL_MONITOR.print(__VA_ARGS__); }
# define DBGLN(...)     { mgSERIAL_MONITOR.println(__VA_ARGS__); }
#else
# define DBG(...)
# define DBGLN(...)
#endif /* DEBUG_USB */

// Character handling
#define	isspace(c)      ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t')
#define	isdigit(c)		((c) >= '0' && (c) <= '9')
#define	tolower(c)		(('A' <= (c) && (c) <= 'Z') ? ((c) + 32) : (c))
#define ascii2hex(a)	(('0' <= (a) && (a) <= '9') ? ((a) - '0') : ((a) - 'a' + 10))

// Types
typedef	struct COMMAND_HANDLER_TABLE_ {	// Command handler table
	char		letter1, letter2;	// first letter and second letter
	uint16_t	flags;				// CHT_FLAG..
	int			(*handler)(char *request, char *response, int *nbytes);
} COMMAND_HANDLER_TABLE;
  // flags
#define	CHT_FLAG_SLEEP_OK	0x0001			// The command that is executable in sleep

  // http functions
extern int _doHttpGet(char *request, char *response, int *nbytes);
extern int _doHttpPost(char *request, char *response, int *nbytes);
  // tcp functions
extern int _connectTCP(char *request, char *response, int *nbytes);
extern int _disconnectTCP(char *request, char *response, int *nbytes);
extern int _readTCP(char *request, char *response, int *nbytes);
extern int _writeTCP(char *request, char *response, int *nbytes);
extern int _getStatusTCP(char *request, char *response, int *nbytes);
extern int _getNameTCP(char *request, char *response, int *nbytes);
extern int _configureTCP(char *request, char *response, int *nbytes);
  // udp functions
extern int _beginUDP(char *request, char *response, int *nbytes);
extern int _sendUDP(char *request, char *response, int *nbytes);
extern int _endUDP(char *request, char *response, int *nbytes);
extern int _getNameUDP(char *request, char *response, int *nbytes);
  // profile function
extern int _setProfile(char *request, char *response, int *nbytes);
  // system functions
extern int _controlLed(char *request, char *response, int *nbytes);
extern int _getIMEI(char *request, char *response, int *nbytes);
extern int _getVersion(char *request, char *response, int *nbytes);
extern int _getTime(char *request, char *response, int *nbytes);
extern int _getRSSI(char *request, char *response, int *nbytes);
extern int _controlAirplaneMode(char *request, char *response, int *nbytes);

#endif // _gw_ltem_h_
