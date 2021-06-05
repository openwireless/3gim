/*
 *  hl7800_udp.cpp
 *
 *  Control library for HL7800 (UDP function)
 *
 *  R0  2020/02/16 (A.D)
 *  R1  2020/06/21 (A.D)
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#include "hl7800.h"

// Symbols


/**
 *  @fn
 *
 *  UDP通信の準備を行う
 *
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int  HL7800::beginUDP(void) {
#ifdef DUMMY_TEST
    return (h78SUCCESS);
#endif

    // Check that session is not exist
    if (_udpSessionId != 0)
        return (h78ERR_ALREADY_INITIALIZED);

    // Configure UDP connection and set udp session id
    h78SENDFLN("AT+KUDPCFG=1,0");
    int stat;
    if ((stat = getSessionId(h78TIMEOUT_LOCAL, "+KUDPCFG:", &_udpSessionId)) != h78SUCCESS) {
        h78USBDPLN("+>KUDPCFG NG: ", stat);
        return (h78ERR_UDP_CONFIG);       // +KUDPCFG error
    }
    h78USBDPLN("+>KUDPCFG OK");

    if ((stat = waitUntilReady(h78TIMEOUT_LOCAL, "+KUDP_IND:", _udpSessionId)) != h78SUCCESS) {
        h78USBDPLN("+>KUDP_IND NG: ", stat);
        return (h78ERR_UDP_CONFIG);       // +KUDPCFG error
    }
    h78USBDPLN("+>KUDP_IND OK");

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  UDP通信を終了する
 *
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int  HL7800::endUDP(void) {
#ifdef DUMMY_TEST
    return (h78SUCCESS);
#endif

    // Check that session is exist
    if (_udpSessionId == 0)
        return (h78ERR_NOT_YET_INITIALIZED);

    // Close udp session and delete it (ignore errors)
    h78SENDFLN("AT+KUDPCLOSE=%d", _udpSessionId);  // <keep_cfg>=0(delete the session configuration)
    discardResponse(300);
    h78SENDFLN("AT+KUDPDEL=%d", _udpSessionId);
    discardResponse(200);

    _udpSessionId = 0;      // Clear udp session id

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  UDP通信でデータを送信する
 *
 *  @param(host)        [in] 通信相手のホスト（IPアドレス(v4)、またはホスト名を指定）
 *  @param(port)        [in] 通信相手のUDPポート番号
 *  @param(msg)         [in] 送信するデータ(バイナリデータ可、ただしEODパターンを含まないこと)
 *  @param(size)        [in] msgのサイズ[Bytes]
 *  @return             0:成功時、0以外:エラー時
 *  @detail             UDPであるため、確実な送信は保証されない(送れなくてもエラーにはならない)
 */
int  HL7800::sendUDP(char *host, int port, void *msg, int size) {
#ifdef DEBUG_USB
    h78USBDPLN("sendUDP(<as follows>, %d):", size);
    uint8_t *p = (uint8_t *)msg;
    for (int i = 0; i < size; i++) {
        h78USBDPFMT((uint8_t)*p++, HEX);
        h78USBDPC(' ');
        if (i % 32 == 31)
            h78USBDPLN("");
    }
    h78USBDPLN("\"");
#endif

    int stat = 0;

    // Check that session is exist
    if (_udpSessionId == 0)
        return (h78ERR_NOT_YET_INITIALIZED);

    if (size > h78MAX_UDP_PAYLOAD_SIZE)
        return (h78ERR_UDP_TOO_BIG_DATA);

    // Send data to the session
    h78SENDFLN("AT+KUDPSND=%d,\"%s\",%d,%d", _udpSessionId, host, port, size);
    h78USBDPLN("AT+KUDPSND=%d,\"%s\",%d,%d", _udpSessionId, host, port, size);
    if ((stat = waitUntilCONNECT(h78TIMEOUT_UDP)) == h78SUCCESS) {
        // Send data
        h78SEND((uint8_t *)msg, size);
        // Send EOD
        h78SENDF(h78END_PATTERN);
        char response[100];
        int len = sizeof(response) - 1;
        if ((stat = getResponse(h78TIMEOUT_UDP, response, &len)) != h78SUCCESS) {
            h78USBDPLN("+>KUDPSND NG(Response): %d", stat);
            return (h78ERR_UDP_RES);
        }
        h78USBDPLN("+>KUDPSND OK");
    }
    else {
        h78USBDPLN("+>KUDPSND NG(Connect): %d", stat);
        return (h78ERR_UDP_CONNECT);
    }

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  自身のIPアドレス(v4)を取得する
 *
 *  @param(ip)          [out] 自分のv4のIPアドレス("99.99.99.99"形式)
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int  HL7800::getNameUDP(char *ip) {
    if (ip == NULL)
        return (h78ERR_BAD_PARAM);

    // Check that session is exist
    if (_udpSessionId == 0)
        return (h78ERR_NOT_YET_INITIALIZED);

    char ipAddress[h78IP_V4_ADDRESS_LENGTH + 1];
    // Get UDP status
    h78SENDFLN("AT+KCGPADDR=1");		// Specify <cnx_cnf>
    if (parseKCGPADDR(ipAddress) != h78SUCCESS)
        return (h78ERR_UDP_NAME);

    h78USBDPLN("+>KCGPADDR: %s", ipAddress);
    strcpy(ip, ipAddress);

    return (h78SUCCESS);
}

// End of hl7800_udp.cpp
