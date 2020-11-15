/*
 *  hl7800_tcp.cpp
 *
 *  Control library for HL7800 (TCP function)
 *
 *  R0  2020/02/16 (A.D)
 *  R1  2020/06/21 (A.D)
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#include "hl7800.h"

/*
 * Symbols
 */

/**
 *  @fn
 *
 *  通信相手にTCPで接続する
 *
 *	@param(host)		[in] 接続する相手のホスト
 *  @param(port)		[in] 接続先のポート番号
 *  @return             0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int	HL7800::connectTCP(const char *host, int port) {
	int stat = h78SUCCESS;
    // Check parameters
	if (port < 0 || port > h78MAX_PORT_NUMBER)
        return (h78ERR_BAD_PARAM);   	// port number error

	if (strlen(host) > h78MAX_HOST_LENGTH)
		return (h78ERR_BAD_PARAM);		// too long host name

	// Check that session is not exist
	if (_tcpSessionId != 0)
		return (h78ERR_TCP_ALREADY_CONNECTED);	// Already connected

	// Connect to host
	h78SENDFLN("AT+KTCPCFG=1,0,\"%s\",%d", host, port);
	if ((stat = getSessionId(h78TIMEOUT_LOCAL, "+KTCPCFG:", &_tcpSessionId)) == h78SUCCESS) {
        h78USBDPLN("+>KTCPCFG OK");

        // Try to connect and wait until TCP connection is ready
        h78SENDFLN("AT+KTCPCNX=%d", _tcpSessionId);
    	if ((stat = waitUntilReady(_timeoutTcpConnect, "+KTCP_IND:", _tcpSessionId)) == h78SUCCESS) {
            h78USBDPLN("+>KTCP_IND OK");
            return (h78SUCCESS);    // OK
        }

        stat = h78ERR_TCP_CONNECT;
		h78USBDPLN("+>KTCP_IND *,1 Not Found: ", stat);
    }
    else {
        stat = h78ERR_TCP_CONFIG;
        h78USBDPLN("+>KTCPCFG NG: ", stat);
    }

	// Close tcp session and delete it (ignore errors)
	h78SENDFLN("AT+KTCPCLOSE=%d", _tcpSessionId);
	discardResponse(500);
    h78SENDFLN("AT+KTCPDEL=%d", _tcpSessionId);
    discardResponse(500);

	// Clear tcp session id
	_tcpSessionId = 0;


	return (stat);	// Something error
}

/**
 *  @fn
 *
 *  通信相手からTCP接続を切断する
 *
 *	@param				なし
 *  @return             0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int	HL7800::disconnectTCP(void) {
	int	stat = h78SUCCESS;
	char response[30];

	// Check that session is exist
	if (_tcpSessionId == 0)
        return (h78ERR_TCP_NOT_YET_CONNECTED);

	// Close tcp session
    h78SENDFLN("AT+KTCPCLOSE=%d", _tcpSessionId);
	int len = sizeof(response) - 1;
	if ((stat = getResponse(h78TIMEOUT_LOCAL, response, &len)) != h78SUCCESS) {
		h78USBDPLN("+>KTCPCLOSE NG: ", stat);
		// Throuh if the error was happened
	}
	h78USBDPLN("+>KTCPCLOSE Pass");

	// Delete tcp session
    h78SENDFLN("AT+KTCPDEL=%d", _tcpSessionId);
	len = sizeof(response) - 1;
	if ((stat = getResponse(h78TIMEOUT_LOCAL, response, &len)) != h78SUCCESS) {
		h78USBDPLN("+>KTCPDEL NG: ", stat);
        stat = h78ERR_TCP_DELETE;
    }
	h78USBDPLN("+>KTCPDEL OK");

	_tcpSessionId = 0;		// Clear tcpSessionId

  	return (stat);
}

/**
 *  @fn
 *
 *  TCPコネクションからデータを読み出す
 *
 *	@param(buf)			[out] 読み出したデータの格納先
 *	@param(size)		[in] bufのサイズ[Bytes]
 *  @return             0:データなし、0～:成功時(読み出したバイト数)、～0:エラー時(エラー番号のマイナス値)
 *  @detail
 */
int	HL7800::readTCP(void *buf, int size) {
	int	stat = h78SUCCESS;

	// Check parameters
    if (size <= 0 || size > h78MAX_TCP_READ_SIZE || buf == NULL)
        return (- h78ERR_BAD_PARAM);

	// Check that session is exist
	if (_tcpSessionId == 0)
        return (- h78ERR_TCP_NOT_YET_CONNECTED);

	// Get TCP status
	int requestBytes = size, status = -1, tcpNotif = 0, receivedBytes = 0;
	h78SENDFLN("AT+KTCPSTAT=%d", _tcpSessionId);
	if ((stat = parseKTCPSTAT(&status, &tcpNotif, NULL, &receivedBytes)) == h78SUCCESS) {
		switch (status) {
		  case 0 :
		  case 3 :
		  case 4 :
		  case 5 :
			// status ok
			break;
		  default :
			// illegal status
			h78USBDPLN("+>KTCPSTAT status: ", status);
			return (- h78ERR_TCP_STATUS);
		}
		if (receivedBytes == 0) {
			// no received data
			return (0);
		}
		else if (requestBytes > receivedBytes)
			requestBytes =  receivedBytes;
	}
	else {
		h78USBDPLN("+>KTCPSTAT NG: ", stat);
		return (- h78ERR_TCP_STATUS);
	}

	// Receive data from the connection
    h78SENDFLN("AT+KTCPRCV=%d,%d", _tcpSessionId, requestBytes);
	if (waitUntilCONNECT(h78TIMEOUT_WRITE) == h78SUCCESS) {
		int len = requestBytes;
		if ((stat = getData(h78TIMEOUT_WRITE, (char *)buf, &len)) != h78SUCCESS) {
			h78USBDPLN("getData() NG: %d", stat);
			return (- h78ERR_TCP_READ);
		}
		h78USBDPLN("+>KTCPRCV OK: %d", len);
		return (len);		// OK
	}
	else {
		// Can't connect..
		return (- h78ERR_TCP_CONNECT);
	}
}

/**
 *  @fn
 *
 *  TCPコネクションへデータを書き出す
 *
 *	@param(buf)			[in] 読み出したデータの格納先
 *	@param(size)		[in] bufに格納されているデータのサイズ[Bytes]
 *  @return             0:データなし、0～:成功時(読み出したバイト数)、～0:エラー時(エラー番号のマイナス値)
 *  @detail
 */
int	HL7800::writeTCP(const void *buf, int size) {
	int stat = h78SUCCESS;

	// Check parameters
	if (size <= 0 || size > h78MAX_TCP_WRITE_SIZE || buf == NULL)
		return (- h78ERR_BAD_PARAM);

	// Check that session is exist
	if (_tcpSessionId == 0)
		return (- h78ERR_TCP_NOT_YET_CONNECTED);

	// Send data to the connection
	h78SENDFLN("AT+KTCPSND=%d,%d", _tcpSessionId, size);
	if ((stat = waitUntilCONNECT(_timeoutTcpWrite)) != h78SUCCESS) {
		h78USBDPLN("+>KTCPSND NG: %d", stat);
		return (- h78ERR_TCP_WRITE);
	}

	// Send data
	h78SEND((uint8_t *)buf, size);
	// and end pettern
	h78SENDF(h78END_PATTERN);
	char resp[20];
	int len = sizeof(resp) - 1;
	if ((stat = getResponse(_timeoutTcpWrite, resp, &len)) != h78SUCCESS) {
		h78USBDPLN("+>KTCPSND NG: %d", stat);
		return (- h78ERR_TCP_WRITE);
	}
	h78USBDPLN("+>KTCPSND OK: %d", size);

	return (size);	// OK
}

/**
 *  @fn
 *
 *  TCPコネクションへデータを直接書き出す
 *
 *	@param(size)		[in] 書き出したいデータのサイズ[Bytes]
 *  @return             0:成功時、0～:エラー時(エラー番号)
 *  @detail				呼び出し側は、本関数が成功した後に h78SERIALポートに対して所定のバイト数sizeのデータを直接書き込む
 *						データを複数回に分けて書き込む場合は、一定以上の間を空けないこと。
 */
int	HL7800::writeBurstTCP(int size) {
	int stat = h78SUCCESS;

	// Check parameters
	if (size <= 0 || size > h78MAX_TCP_DATA_SIZE_ONCE)
		return (- h78ERR_BAD_PARAM);

	// Check that session is exist
	if (_tcpSessionId == 0)
		return (- h78ERR_TCP_NOT_YET_CONNECTED);

	// Send data to the connection
	h78SENDFLN("AT+KTCPSND=%d,%d", _tcpSessionId, size);
	if ((stat = waitUntilCONNECT(h78TIMEOUT_WRITE)) == h78SUCCESS) {
		h78USBDPLN("+>KTCPSND NG: %d", stat);
		return (h78ERR_TCP_WRITE);
	}

	return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  TCPコネクションのステータスを調べる
 *
 *	@param(status)			[out] TCPコネクションのステータス(シンボルh78TCPSTAT_*を参照)
 *  @param(tcpNotif)		[out] TCPコネクションの通知情報(下記参照)
 *                            0 Network error
 *	                          1 No more sockets available; max. number alreadyreached
 *	                          2 Memory problem
 *	                          3 DNS error
 *	                          4 TCP disconnection by the server or remote client
 *                            5 TCP connection error
 *                            6 Generic error
 *                            7 Fail to accept client request’s
 *                            8 Data sending is OK but KTCPSND was waiting more or less characters
 *                            9 Bad session ID
 *                            10 Session is already running
 *                            11 All sessions are used
 *                            12 Socket connection timeout error
 *                            13 SSL connection error
 *                            14 SSL initialization error
 *  @param(remainedBytes)	[out] TCPコネクションに残っている、未送信のデータのサイズ[Bytes]
 *  @param(receivedBytes)	[out] TCPコネクションに残っている、受信済みのデータのサイズ[Bytes]
 *  @return             	0:成功時、0～:エラー時(エラー番号)
 *  @detail
 */
int	HL7800::getStatusTCP(int *status, int *tcpNotif, int *remainedBytes, int *recievedBytes) {
	int	st, stat = h78SUCCESS;

	// Pre-set out parameters
	*status = 0;
	*tcpNotif = 0;
	*remainedBytes = *recievedBytes = 0;

	// Check that session is exist
	if (_tcpSessionId == 0)
		return (- h78ERR_TCP_NOT_YET_CONNECTED);

	// Get TCP status
	h78SENDFLN("AT+KTCPSTAT=%d", _tcpSessionId);
	if ((stat = parseKTCPSTAT(&st, tcpNotif, remainedBytes, recievedBytes)) != h78SUCCESS) {
		h78USBDPLN("+>KTCPSTAT NG: %d", stat);
		return (h78ERR_TCP_STAT);
	}

	switch (st) {
      case 0 :	   // Not defined
        *status = h78TCPSTAT_NOT_DEFINED;
        break;
	  case 1 :     // Disconnected
		*status = h78TCPSTAT_DISCONNECTED;
		break;
	  case 2 :	    // Connecting
		*status = h78TCPSTAT_CONNECTING;
		break;
	  case 3 :     	// Connected
		*status = h78TCPSTAT_CONNECTED;
		break;
	  case 4 :     // Closing, wait for status 5
	  case 5 :     // Closed
		*status = h78TCPSTAT_CLOSED;
		break;
	  default :    	// Unknown status
		*status = h78TCPSTAT_UNKNOWN;
		break;
	}

	return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  自分のIPアドレスを取得する
 *
 *	@param(ip)			[out] 自分のv4のIPアドレス("99.99.99.99"形式)
 *  @return             0:成功時、0～:エラー時(エラー番号)
 *  @detail				ipは最低h78IP_V4_ADDRESS_LENGTHバイト以上のサイズがあること
 */
int	HL7800::getNameTCP(char *ip) {
	int	stat = h78SUCCESS;

	// Check that session is exist
	if (_tcpSessionId == 0)
		return (- h78ERR_TCP_NOT_YET_CONNECTED);

	// Get TCP status
	char ipAddress[h78IP_V4_ADDRESS_LENGTH+1];
	h78SENDFLN("AT+KCGPADDR=1");		// Specify <cnx_cnf>
	if ((stat = parseKCGPADDR(ipAddress)) != h78SUCCESS) {
		h78USBDPLN("+>KCGPADDR NG: %d", stat);
		return (h78ERR_TCP_ADDR);
	}
	strcpy(ip, ipAddress);

	return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  TCP機能使用するタイマのタイムアウト値を変更する
 *
 *	@param(timeout_connect)		[in] connectTCP()で使用するタイムアウト値
 *	@param(timeout_write) 		[in] writeTCP()で使用するタイムアウト値
 *  @return             0:成功時、0～:エラー時(エラー番号)
 *  @detail				設定した値はSRAM上に保存するため、電源Offやリセットでは維持されない
 */
int HL7800::configureTCP(uint32_t timeoutConnect, uint32_t timeoutWrite) {
    // Check parameters
    if (timeoutConnect < 0 || timeoutConnect > h78TIMEOUT_CONNECT * 3)
        return (h78ERR_BAD_PARAM);
    if (timeoutWrite < 0 || timeoutWrite > h78TIMEOUT_WRITE * 3)
        return (h78ERR_BAD_PARAM);

    // Set tcp parameters
    _timeoutTcpConnect = timeoutConnect;
    _timeoutTcpWrite = timeoutWrite;

	return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  KCGPADDR:からIPアドレスを取得する
 *
 *	@param(ipAddress)	[out] 取得したIPアドレス
 *  @return             0:成功時、0～:エラー時(エラー番号)
 *  @detail
 */
int	HL7800::parseKCGPADDR(char *ipAddress) {
	uint32_t limit = millis() + h78TIMEOUT_LOCAL;

	while (true) {
		char line[h78IP_V4_ADDRESS_LENGTH + 20];
		int len;
		if ((len = getLine(limit, line, sizeof(line))) == 0)
			break;		// Timed out

		// (ex.) +KCGPADDR: 1,"192.168.1.49"
		if (len > 10 && ! strncmp(line, "+KCGPADDR:", 10)) {
			h78USBDPLN("+KCGPADDR>");
			h78USBDPWRT(line, len);
			h78USBDPLN("<");

			for (char *p = line + 14; *p != '"'; )
				*ipAddress++ = *p++;
			*ipAddress = '\0';		// terminate string

			return (h78SUCCESS);
		}
		else if (len > 5 && ! strncmp(line, "ERROR", 5)) {
			h78USBDPLN(">ERROR");
			return (h78ERR_TCP_ADDR);
		}
	}

	h78USBDPLN("T/O>");
	return (h78ERR_TIMED_OUT);
}

/*
 * Local functions
 */


/**
 *  @fn
 *
 *  KCGPADDR:からIPアドレスを取得する
 *
 *	@param(ipAddress)	[out] 取得したIPアドレス
 *  @return             0:成功時、0～:エラー時(エラー番号)
 *  @detail
 */
/*
 * +KTCPSTAT: status,tcpNotif,remainedBytes,receivedBytes\r\n
 *   status: value to indicate TCP socket state
 *		0 socket not defined, use KTCPCFG to create a TCP socket
 *		1 socket is only defined but not used
 *		2 socket is opening and connecting to the server, cannot be used
 *		3 connection is up, socket can be used to send/receivedata
 *		4 connection is closing, it cannot be used, wait for status 5
 *		5 socket is closed
 *   tcpNotif: -1 if socket/connection is OK , <tcp_notif> if an error has happened
 *   remainedBytes: remaining bytes in the socket buffer, waiting to be sent
 *   receibedBytes: received bytes, can be read with +KTCPRCV command
 *
 */
int	HL7800::parseKTCPSTAT(int *status, int *tcpNotif, int *remainedBytes, int *recievedBytes)
{
	uint32_t limit = millis() + h78TIMEOUT_LOCAL;

	while (true) {
		char line[30];
		int len;
		if ((len = getLine(limit, line, sizeof(line))) == 0)
			break;		// Timed out

		// (ex.) +KTCPSTAT: 3,-1,0,0
		if (! strncmp(line, "+KTCPSTAT: ", 11)) {
			*status = atoi(line+11);
			h78USBDPLN("+KTCPSTAT>\n");
			h78USBDPWRT(line, len);
			h78USBDPLN("<");
			h78USBDPLN(">status=%d", *status);
			int	nthComma = 0;
			for (int i = 11; i < len; i++) {
				if (line[i] == ',') {
					nthComma++;
					if (nthComma == 1) {
						int n = atoi(line + i + 1);
						if (tcpNotif != NULL)
							*tcpNotif = n;
						h78USBDPLN(">tcpNotif=%d", n);
					}
					else if (nthComma == 2) {
						int n = atoi(line + i + 1);
						if (remainedBytes != NULL)
							*remainedBytes = n;
						h78USBDPLN(">remBytes=%d", n);
					}
					else {	// nthComma == 3
						int n = atoi(line + i + 1);
						if (recievedBytes != NULL)
							*recievedBytes = n;
						h78USBDPLN(">rcvBytes=%d", n);
					}
				}
			}

			return (h78SUCCESS);
		}
		else if (! strncmp(line, "ERROR", 5)) {
			h78USBDPLN(">ERROR");
			return (h78ERR_INTERNAL_ERROR);
		}
	}
	h78USBDPLN("T.O>");

	return (h78ERR_TIMED_OUT);
}

/**
 *  @fn
 *
 *  シリアルからデータを読み出す
  *
 *	@param(timeout)		[in]
 *  @param(resp)		[out]
 *	@param(size)		[in/out]
 *  @return             0:成功時(常に)
 *  @detail
 */
int HL7800::getData(uint32_t timeout, char *resp, int *size) {
	uint32_t limit = millis() + timeout;
	int length = 0;
	int c;
	while (millis() < limit && length < *size) {
		if ((c = h78SERIAL.read()) < 0)
            continue;       // Skip
		*resp++ = (char)c;
		length++;
	}
    *size = length;

    if (length < *size)
        return (h78ERR_TIMED_OUT);

    return (h78SUCCESS);    // end of input
}

/*
 * KCNX_IND:
 * 		Disconected					KCNX_IND: <cnx_cnf>,0,<af>
 * 		Connected					KCNX_IND: <cnx_cnf>,1,<af>
 * 		Failed to connect			KCNX_IND: <cnx_cnf>,2,<attempt>,<nbtrial>,<tim1>
 * 		Closed						KCNX_IND: <cnx_cnf>,3
 * 		Connecting					KCNX_IND: <cnx_cnf>,4,<attempt>
 * 		Idle time counting start	KCNX_IND: <cnx_cnf>,5,<idletime>
 * 		Idle time counting canceled	KCNX_IND: <cnx_cnf>,6
 *
 * 	where
 * 		<af> = 0(IPv4) or 1(IPv6)
 * 		<attempt> = Current attempt of bringing up of PDP connection
 * 		<nbtrial> =
 * 		<tim1> =
 * 		<idletime> =
 *
 */

// End of hl7800_tcp.cpp
