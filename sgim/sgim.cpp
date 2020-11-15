
/*
 *	sgim.cpp -- SGIM(V1) Control Class Library (implmentation)
 *
 *	History:
 *		R0.1 2018/11/24  1st Release for SGIM(Ver1.0)
 *      R0.2 2019/01/07  fix sleep(), add isActive()
 *      R0.3 2019/03/09  fix sendBuffer()
 *      R0.5 2019/04/14  change sgTIMEOUT_UPLINK(10sec. -> 15Sec.)
 *      R0.6 2019/12/23  change rebootSigfoxModule(), fix timeout bug in sendBuffer()
 *      R0.7 2020/08/10  add getVBAT(), change settings of analogReferense() and analogResolution()
 *      &0.8 2020/09/21  fix getVBAT()
 *
 *	Author:
 *		Open wireless Alliance and TABrain Inc.
 */

#include <sgim.h>

// Symbols
#define	SEND_INTERVAL	10000UL					//
#define END_OF_RESPONSE '\r'  					//  Character '\r' marks the end of response.
  // Command strings
#define CMD_END 					"\r"
#define CMD_AT 						"AT"
#define CMD_SEND_MESSAGE 			"AT$SF="  	//  Prefix to send a message to SIGFOX cloud.
#define CMD_SEND_MESSAGE_RESPONSE 	",1"  		//  Expect downlink response from SIGFOX.
#define CMD_GET_ID 					"AT$I=10"  	//  Get SIGFOX device ID.
#define CMD_GET_PAC 				"AT$I=11"  	//  Get SIGFOX device PAC, used for registering the device.
#define CMD_GET_TEMPERATURE 		"AT$T?"  	//  Get the module temperature.
#define CMD_GET_VOLTAGE 			"AT$V?"  	//  Get the module voltage.
#define CMD_RESET 					"AT$P=0"  	//  Software reset.
#define CMD_SLEEP 					"AT$P=1"  	//  Switch to sleep mode : consumption is < 1.5uA
#define CMD_WAKEUP 					"AT$P=0"	//  Switch back to normal mode : consumption is 0.5 mA @change
#define CMD_RCZ3 					"AT$IF=902080000"  //  JP / RCZ3 Frequency

// Define SGIM instance
SGIM	sgim;

//
bool SGIM::begin(void) {
	if (_hasInitialized)
		return false;

	_hasInitialized = true;
	_lastSent = 0;

	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, LOW);	// turn off
	pinMode(interruptPin, INPUT_PULLUP);
//	pinMode(1, OUTPUT);
//	digitalWrite(1, HIGH);		// External TX1 as high

	// Set up Sigfox module
	pinMode(sigfoxResetPin, OUTPUT);
	digitalWrite(sigfoxResetPin, HIGH);

    // Configures the reference voltage as 2.23V and resolution as 12-bit
    analogReadResolution(12);
    analogReference(AR_INTERNAL2V23);

	return true;
}

//
bool SGIM::end(void) {
	if (! _hasInitialized)
		return false;

	_hasInitialized = true;

	return true;
}

// Reboot Sigfox module
bool SGIM::rebootSigfoxModule(boolean forced) {
	if (forced) {
		// Reset-pinを使って、強制的にリセットを掛ける
		digitalWrite(sigfoxResetPin, LOW);
		delay(100);
		digitalWrite(sigfoxResetPin, HIGH);
		delay(200);
	}
	else {
		// RESETコマンドを送って、リセットを掛ける
		String data;
		uint8_t markers;
		if (! sendCommand(String(CMD_RESET) + CMD_END, 1, data, markers))
			return false;
	}

	return true;
}

// Reset SAMD21G18
void SGIM::resetBoard(void) {
	NVIC_SystemReset();
}

void SGIM::setLed(int onoff) {
	digitalWrite(ledPin, onoff);
}

int SGIM::onDetectMotion(void (*handler)(void)) {
	_onMotionHandler = handler;
	if (handler)
		attachInterrupt(interruptPin, handler, LOW);
	else
		detachInterrupt(interruptPin);
}

int SGIM::getVBAT(void) {
    const int numberOfMeasurements = 10;
    int total = 0;
    for (int i = 0; i < numberOfMeasurements; i++) {
        int voltage = analogRead(vbatPin) * 0.4340;  // reference = 2.23V
        total += voltage * 4;
        delay(10);
    }
    return (total / numberOfMeasurements);
}

bool SGIM::getID(String &id) {
	String data;
	uint8_t markers;
	if (! sendCommand(String(CMD_GET_ID) + CMD_END, 1, data, markers))
		return false;
	id = data;

	return true;
}

bool SGIM::getPAC(String &pac) {
	String data;
	uint8_t markers;
	if (! sendCommand(String(CMD_GET_PAC) + CMD_END, 1, data, markers))
		return false;
	pac = data;

	return true;
}

bool SGIM::sendMessage(const String &payload) {
	if (! readyToSend())
		return false;  	//  Prevent user from sending too many messages.

	uint8_t markers;
	String data, message = String(CMD_SEND_MESSAGE) + payload + CMD_END;
	if (sendBuffer(message, sgTIMEOUT_UPLINK, 1, data, markers)) {  //  One '\r' marker expected ("OK\r").
		_lastSent = millis();
		return true;	// OK
	}

	return false;
}

bool SGIM::sendMessageWithResponse(const String &payload, String &response) {
	if (! readyToSend())
		return false;  	//  Prevent user from sending too many messages.

	uint8_t markers;
	String data, message = String(CMD_SEND_MESSAGE) + payload + CMD_SEND_MESSAGE_RESPONSE + CMD_END;
	//  Two '\r' markers expected ("OK\r RX=...\r").
	if (sendBuffer(message, sgTIMEOUT_DOWNLINK, 2, data, markers)) {
		_lastSent = millis();
		response = data;
		//  Response contains OK\nRX=01 23 45 67 89 AB CD EF
		//  Remove the prefix and spaces.
		response.replace("OK\nRX=", "");
		response.replace(" ", "");
		return true;	// OK
	}

	return false;
}

bool SGIM::readyToSend(void) {
	if (_lastSent == 0)
		return true;
	else if (millis() - _lastSent > SEND_INTERVAL)
		return true;

	return false;
}

bool SGIM::enterCommandMode(void) {
	return true;
}

bool SGIM::exitCommandMode(void) {
	return true;
}

bool SGIM::sleep(void) {
	String data;
	uint8_t markers;

	// Check UART condition
	sendCommand(String(CMD_AT) + CMD_END, 1, data, markers);

	// Sleep command
	if (! sendCommand(String(CMD_SLEEP) + CMD_END, 1, data, markers))
		return false;

	return true;	// OK
}

bool SGIM::wakeup(void) {
	sgSerial.begin(sgBAUDRATE_SIGFOX);
	sgSerial.write('\r');
	delay(30);
//	sgSerial.flush();
	sgSerial.end();

	delay(100);

	return true;	// OK
}

bool SGIM::isActive(void) {
	return (digitalRead(sigfoxCPULedPin));
}

bool SGIM::getVoltage(int &voltage) {
	String data;
	uint8_t markers;
	if (! sendCommand(String(CMD_GET_VOLTAGE) + CMD_END, 1, data, markers))
		return false;
	voltage = data.toInt();		// Unit is "mV"

	return true;	// OK
}

bool SGIM::getTemperature(float &temperature) {
	String data;
	uint8_t markers;
	if (! sendCommand(String(CMD_GET_TEMPERATURE) + CMD_END, 1, data, markers))
		return false;
	temperature = data.toInt() / 10.0;

	return true;	// OK
}

String SGIM::toHex(uint8_t u8) {
	//  Convert the char to a string of 2 hex digits.
	uint8_t *b = (uint8_t *)&u8;
	String bytes;
	if (b[0] <= 0x0f)
		bytes.concat('0');
	bytes.concat(String(b[0], 16));

	return bytes;
}

String SGIM::toHex(uint16_t u16) {
	//  Convert the integer to a string of 4 hex digits.
	uint8_t *b = (uint8_t *)&u16;
	String bytes;
	for (int i = 0; i < 2; i++) {
		if (b[i] <= 0x0f)
			bytes.concat('0');
		bytes.concat(String(b[i], 16));
	}

	return bytes;
}

String SGIM::toHex(uint32_t u32) {
	//  Convert the long to a string of 8 hex digits.
	uint8_t * b = (uint8_t *)&u32;
	String bytes;
	for (int i = 0; i < 4; i++) {
		if (b[i] <= 0x0f)
			bytes.concat('0');
		bytes.concat(String(b[i], 16));
	}

	return bytes;
}

String SGIM::toHex(char *s) {
	//  Convert the string to a string of hex digits.
	uint8_t *b = (uint8_t *)s;
	String bytes;
	for (int i = 0; i < strlen(s); i++) {
		if (b[i] <= 0x0f)
			bytes.concat('0');
		bytes.concat(String(b[i], 16));
	}

	return bytes;
}

String SGIM::toHex(float f) {
	uint8_t *b = (uint8_t *)&f;
	String bytes;
	for (int i = 0; i < 4; i++) {
		if (b[i] <= 0x0f)
			bytes.concat('0');
		bytes.concat(String(b[i], 16));
	}

	return bytes;
}

uint8_t SGIM::toUint8(String s) {
  char high = s.charAt(0);
  char low = s.charAt(1);
  return ((toHexdecimal(high) << 4) + toHexdecimal(low));
}

uint16_t SGIM::toUint16(String s) {
  char high0 = s.charAt(0);
  char low0 = s.charAt(1);
  char high1 = s.charAt(2);
  char low1 = s.charAt(3);
  return (((uint16_t)toHexdecimal(high1) << 12) + ((uint16_t)toHexdecimal(low1) << 8)
          + ((uint16_t)toHexdecimal(high0) << 4) + (uint16_t)toHexdecimal(low0));
}

uint32_t SGIM::toUint32(String s) {
  uint32_t low = toUint16(s);
  uint32_t high = toUint16(s.substring(4));
  return ((high << 16) + low);
}

/*
 *	Private methods
 */
bool SGIM::sendBuffer(const String &buffer, const uint32_t timeout, uint8_t expectedMarkerCount, String &response, uint8_t &actualMarkerCount) {
	response = "";
	actualMarkerCount = 0;

	sgSerial.begin(sgBAUDRATE_SIGFOX);
	sgSerial.flush();
    delay(10);
	while (sgSerial.available() > 0)
		(void)sgSerial.read();

	const char *rawBuffer = buffer.c_str();
	unsigned long startTime = millis();
	int i = 0;
	for (;;) {
		//  If there is data to send, send it.
		if (i < buffer.length()) {
			uint8_t txChar = rawBuffer[i++];
			sgSerial.write(txChar);
			delay(2);
			startTime = millis();  //  Start the timer only when all data has been sent.
		}
		//  If timeout, quit.
		uint32_t currentTime = millis();
		if (currentTime - startTime > timeout)
			break;

		//  If data is available to receive, receive it.
		if (sgSerial.available() > 0) {
			int rxChar = sgSerial.read();
			if (rxChar == -1)
				continue;
			if (rxChar == END_OF_RESPONSE) {
				if (actualMarkerCount < markerPosMax)
					_markerPos[actualMarkerCount] = response.length();  //  Remember the marker pos.
				actualMarkerCount++;  //  Count the number of end markers.
				if (actualMarkerCount >= expectedMarkerCount)
					break;  //  Seen all markers already.
			}
			else {
				response.concat(String((char)rxChar));
			}
		}
	}

//	sgSerial.flush();
	sgSerial.end();

	//  If we did not see the terminating '\r', something is wrong.
	if (actualMarkerCount < expectedMarkerCount) {
		return false;	// No response or bad response
	}

	return true;	// OK
}

bool SGIM::sendCommand(const String &cmd, uint8_t expectedMarkerCount, String &result, uint8_t &actualMarkerCount) {
	String data;
	if (! sendBuffer(cmd, sgTIMEOUT_LOCAL, expectedMarkerCount, data, actualMarkerCount))
		return false;

	result = data;
	return true;
}

uint8_t SGIM::toHexdecimal(char d) {
    if ('0' <= d && d <= '9')
        return (d - '0');
    else
        return ((d - 'A') + 10);
}
