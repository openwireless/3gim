/*
 *  gw_ltem.ino - Command interpreter for mgim
 *
 *  R0  2020.03.09 605e
 */

#include "gw_ltem.h"

// Buffer
char	request[MAX_COMMAND_LENGTH+64];		// buffer for $command
char	response[MAX_RESULT_LENGTH+64];		// buffer for $command result

// Command handler table
COMMAND_HANDLER_TABLE chTab[] = {
    // Web commands
 	{ 'W', 'G', 0, _doHttpGet },		// $WG url ["header"]
 	{ 'W', 'P', 0, _doHttpPost },		// $WP url "body" ["header"]
 	// TCP commands
 	{ 'T', 'C', 0, _connectTCP },		// $TC host port
    { 'T', 'D', 0, _disconnectTCP },	// $TD
 	{ 'T', 'R', 0, _readTCP },			// $TR nbytes
 	{ 'T', 'W', 0, _writeTCP },			// $TW "data"
// 	{ 'T', 'T', 0, _writeTunnelTCP },	// $TT nbytes\n..data..<EOF>
 	{ 'T', 'S', 0, _getStatusTCP },		// $TS [1]
 	{ 'T', 'N', 0, _getNameTCP },		// $TN
 	{ 'T', 'X', 0, _configureTCP },		// $TX
 	// UDP commands
 	{ 'U', 'B', 0, _beginUDP },			// $UB
 	{ 'U', 'S', 0, _sendUDP },			// $US host port nbytes\n..data..<EOF>
 	{ 'U', 'E', 0, _endUDP },			// $UE
 	{ 'U', 'N', 0, _getNameUDP },		// $UN
 	// System commands
 	{ 'Y', 'I', 0, _getIMEI },			// $YI
 	{ 'Y', 'L', CHT_FLAG_SLEEP_OK, _controlLed },		// $YL [0/1]
 	{ 'Y', 'R', 0, _getRSSI },			// $YR
// 	{ 'Y', 'S', 0, _getService },		// $YS
 	{ 'Y', 'T', 0, _getTime },			// $YT [1]
 	{ 'Y', 'V', CHT_FLAG_SLEEP_OK, _getVersion },		// $YV
// 	{ 'Y', 'A', 0, _atPassThrough },	// $YA
// 	{ 'Y', 'X', 0, _setNetworkParams},	// $YX t1 t2 idletime	--@add (R1.1)
// 	{ 'Y', 'E', CHT_FLAG_SLEEP_OK, _resetMyself },			// $YE
// 	{ 'Y', 'B', CHT_FLAG_SLEEP_OK, _controlBaudrate },		// $YB [baudrate]
 	{ 'Y', 'P', CHT_FLAG_SLEEP_OK, _controlAirplaneMode },	// $YP [0/1] -- can execute in airplane mode
 	// Profile commands
 	{ 'P', 'S', 0, _setProfile },		// $PW [apn user password]  -- not compatible with older version(v1)
 	// End mark
 	{ '\0', '\0', 0, NULL }				// - TERMINATOR(END OF TABLE ENTRIES) -
};
#define	CHTAB_NUM		(sizeof(CHTab) / sizeof(struct COMMAND_HANDLER_TABLE))

int		ledStatus 			= 0;		// Green LED Status (0:off/1:on)
int 	airplaneMode 		= 0;		// Airplane Mode (0:off/1:on)
int		baudrate0			= 0;		// Current UART0 baudrate (9600/19200/38400/57600/115200)
  // network parameters
int		time1				= 30;		// Retry interval of PDP context connection [Sec.]
int		time2 				= 60;		// Wait time of TCP connection [Sec.]
int		idleTime 			= 30;		// Expire time of idle timer [Sec.]
int		nbtrial				= 2;		// Attempt times activate to PDP context [Times]

// HL7800 instance
HL7800  hl7800;

/*
 * Local functions prototype
 */
int	doCommand(char *request, char *response, int *nbytes);
int getCommand(char *buffer, int size);

/*
 *  setup()
 */
void setup(void) {
    if (mgim.begin() != mgSUCCESS) {
        DBGLN("mgim.begin(): error.");
        while (1) ;       // Stop here
    }

    hl7800.powerOn();
    delay(500);
    int stat = hl7800.begin();
    if (stat != 0) {
        DBGLN("hl7800(): error");
        while (1) ;       // Stop here
    }

    // Open interface serial port
    Serial1.begin(115200);

#ifdef DEBUG_USB
    SerialUSB.begin(115200);
    while (! SerialUSB)
        ;
#endif

    // Welcome to message
    blinkLed(3, 100);
    outputln(WELCOME_MESSAGE);
}

void loop(void) {
    // Command input and execution loop
    int stat, nbytes = MAX_RESPONSE_LENGTH;
    if ((stat = getCommand(request, MAX_COMMAND_LENGTH)) == SUCCESS) {
        stat = doCommand(request, response, &nbytes);
        outputln(response, nbytes);
    }
    else {
        // Command format error
        output("$=NG ");
        switch (stat) {
          case TOO_LONG_COMMAND :
            outputln("Too long");
            break;
          case BAD_FORMAT :
            outputln("Bad format");
            break;
        }
    }

    // Discard input and clear response buffer
    discardInput();
    memset(response, sizeof(response), 0);
}

int doCommand(char *request, char *response, int *nbytes) {
	// search command handler from chTab[]
	COMMAND_HANDLER_TABLE *p;
	int found = 0;
	for (p = chTab; p->letter1 != '\0'; p++) {
		if (request[1] == p->letter1 && request[2] == p->letter2) {
			found = 1;
			break;	// command found
		}
	}

	// Call command handler if found in chTab
	if (found) {
		if (airplaneMode && ! (p->flags & CHT_FLAG_SLEEP_OK)) {
			strcpy(response, "$=NG airplane mode");
			*nbytes = strlen(response);
			return (CANNOT_EXECUTE_IN_AIRPLANEMODE);
		}
		int stat = p->handler(request, response, nbytes);
		return stat;
	}
	else {
		strcpy(response, "$=NG Unknown");
		*nbytes = strlen(response);
		return (BAD_COMMAND);	// BAD_COMMAND
	}
}

int getCommand(char *buffer, int size) {
	int n = 0;
	char c;
	do {
		if (Serial1.available() > 0) {
			c = Serial1.read();
			if (n < size - 1)
				buffer[n++] = c;
			else
				return (TOO_LONG_COMMAND);  	// ERROR_TOO_LONG_REQUEST
		}
	} while (c != '\n');

	buffer[n] = '\0';	// command terminate by \0

	if (buffer[0] != '$')
		return (BAD_FORMAT);	// ERROR_BAD_FORMAT_REQUEST

	return (SUCCESS);
}
