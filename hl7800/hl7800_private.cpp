/*
 *  hl7800_private.cpp
 *
 *  Control library for HL7800 (Private methods)
 *
 *  R0  2020/02/16 (A.D)
 *  R1  2020/06/21 (A.D)  fix parseCGATT()
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#include "hl7800.h"

/**
 *  @fn
 *
 *  指定された時間経過後に送られてきているレスポンスを読み捨てる
 *
 *  @param(timeout)     待つ時間[mS]
 *  @return             なし
 *  @detail
 */
void HL7800::discardResponse(uint32_t timeout) {
    // まず、指定された時間だけ待つ
    delay(timeout);

    // 次に、HL7800から送られてきているレスポンスを読み捨てる
    h78USBDPLN("DISCARD>");
    h78SERIAL.setTimeout(1);    // readByes()のTimeoutは小さくしておく
    while (h78SERIAL.available() > 0) {
        char buf[100];
        int nBytes = h78SERIAL.readBytes(buf, sizeof(buf) - 1);
        buf[nBytes] = '\0';
        h78USBDP(buf);
    }
    h78USBDPLN("<DISCARD");
}

/**
 *  @fn
 *
 *  指定されたインディケータを検出して、セッションIDを取得する
 *
 *  @param(timeout)     [in] タイムアウト時間[mS]
 *  @param(ind)         [in] インディケータの先頭文字列(ex. "XXXX_ID:")
 *  @param(sessionId)   [out] 取得したセッションID
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::getSessionId(uint32_t timeout, char *ind, int *sessionId) {
  // Response patters are as follow:
  //   XXXXX_IND: <session_id>\r\n
  //   XXXXX_IND: <session_id>,..\r\n
  //   ERROR
#ifdef DEBUG_USB
    h78USBDPLN(">getSessionId(%d,\"%s\",-)", timeout, ind);
#endif

    int  indLength = strlen(ind);
    uint32_t limit = millis() + timeout;
    while (true) {
        char line[50];
        int len = sizeof(line) - 1;
        h78SERIAL.setTimeout(limit - millis());
        if ((len = h78SERIAL.readBytesUntil('\n', line, len)) == 0)
            break;    // Timed out
        line[len] = '\0';
        h78USBDPLN("line>>%s<<", line);
        if (len > indLength && ! strncmp(line, ind, indLength)) {
            h78USBDPLN("%s>%s<", ind, line);
            for (int i = indLength; i < len; i++) {
                if (isdigit(line[i])) {
                    *sessionId = atoi(line + i);
                    h78USBDPLN("*SessionId=%d", *sessionId);
                    return (h78SUCCESS);
                }
            }
            return (h78ERR_INTERNAL_ERROR);
        }
        else if (! strncmp(line, "ERROR", 5)) {
            h78USBDPLN("ERROR><");
            return (h78ERR_INTERNAL_ERROR);
        }
    }
    h78USBDPLN("TIMEOUT><");

    return (h78ERR_TIMED_OUT);
}

/**
 *  @fn
 *
 *  接続が完了するまで待つ
 *
 *  @param(timeout)     [in] タイムアウト時間[mS]
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::waitUntilCONNECT(uint32_t timeout) {
#ifdef DEBUG_USB
    h78USBDPLN(">waitUntilCONNECT(%d)", timeout);
#endif

    uint32_t limit = millis() + timeout;
    while (1) {
        char line[30];
        int len = sizeof(line) - 1;
        h78SERIAL.setTimeout(limit - millis());
        if ((len = h78SERIAL.readBytesUntil('\n', line, sizeof(line))) == 0)
            break;    // Timed out
        line[len] = '\0';
        h78USBDPLN("line=\"%s\"", line);
        if (len >= 8 && ! strncmp(line, "CONNECT\r", 8)) {
            h78USBDPLN("<waitUntilCONNECT() OK: %d", timeout - (limit - millis()));
            return (h78SUCCESS);
        }
        else if (len >= 11 && ! strncmp(line, "NO CARRIER\r", 11)) {
            h78USBDPLN("<waitUntilCONNECT() NG");
            return (h78ERR_CANOT_CONNECT);
        }
    }
    h78USBDPLN("<waitUntilCONNECT() T/O");

    return (h78ERR_TIMED_OUT);
}

/**
 *  @fn
 *
 *  AT+CGATTコマンドのレスポンスを取得・解析する
 *
 *  @param(state)       [out] 取得した状態
 *  @return             0:タイムアウト時、1～:成功時(取得したバイト数)
 *  @detail             タイムアウト時間としてh78TIMEOUT_CGATTを使用
 */
int  HL7800::parseCGATT(int *state) {
    // Legal response format:
    // <Insert SIM>
    //    +CGATT: state
    //    OK
    // <No SIM>
    //    ERROR

    *state = 0;

    // State lineを取得して処理する
    char line[30];
    uint32_t limit = millis() + h78TIMEOUT_CGATT;
    while (millis() <= limit) {
        int len = sizeof(line) - 1;
        h78SERIAL.setTimeout(limit - millis());
        if ((len = h78SERIAL.readBytesUntil('\r', line, sizeof(line)-1)) == 0)
            return (h78ERR_TIMED_OUT);

        line[len] = '\0';
        h78USBDPLN(">>%s<<", line);
        // Skip CR, NL or Spaces
        char *p = line;
        while (*p == '\r' || *p == '\n' || *p == ' ')
            p++;

        if (! strncmp(p, "+CGATT:", 7)) {
            *state = atoi(p + 8);
            h78USBDPLN("+>CGATT state=%d", *state);
            break;
        }
        else if (! strncmp(p, "ERROR", 5)) {
            h78USBDPLN("+>CGATT ERROR");
            break;
        }
    }

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  AT+CCLKコマンドのレスポンスを解析する
 *
 *  @param(resp)        [in] 取得した状態
 *  @param(datetime)    [out] 取得した日時(YYYY/MM/DD hh:mm:ss形式)
 *  @return             0:成功時、0以外:失敗時(エラーコード)
 *  @detail             タイムアウト時間としてh78TIMEOUT_CGATTを使用
 */
int HL7800::parseCCLK(char *resp, char *datetime) {
    for (char *p = resp; *p != '\0'; p++) {
        if (*p != '+')
            continue;
        if (strncmp(p, "+CCLK:", 6))
            continue;

        // +CCLK found, extract date and time from +CCLK response
        if (*(p+8) == '0' && *(p+9) == '0')
            return (h78ERR_INVALID_DATETIME);   // "2000/00/00" is invalid date
        strncpy(datetime, "20", 2);
        strncpy(datetime + 2, p + 8, 17);
        datetime[10] = ' ';
        datetime[19] = '\0';

        return (h78SUCCESS);    // +CCLK found
    }

    return (h78ERR_INTERNAL_ERROR);    // +CCLK not found
}

/**
 *  @fn
 *
 *  Ready状態になるまで待つ
 *
 *  @param(timeout)     [in] タイムアウト時間[mS]
 *  @param(prefix)      [in] セッション取得のキーワードとなる文字列
 *  @param(sessionId)   [out] 取得したセッションID(1～MAX_SESSION_ID)
 *  @return             0:成功時、0以外:失敗時(エラーコード)
 *  @detail
 */
int  HL7800::waitUntilReady(uint32_t timeout, char *prefix, int sessionId) {
    uint32_t limit = millis() + timeout;
    char ind[50];
    sprintf(ind, "%s %d,1", prefix, sessionId);
    h78USBDPLN("ind=\"%s\"", ind);
    int  indLength = strlen(ind);
    while (true) {
        char line[sizeof(ind)];
        int len;
        if ((len = getLine(limit, line, sizeof(line))) == 0)
            break;    // Timed out
        if (len > 5 && ! strncmp(line, "ERROR", 5)) {
            h78USBDPLN("<ERROR");
            return (h78ERR_ERROR);
        }
        if (len > indLength && ! strncmp(line, ind, indLength)) {
            h78USBDPLN("%s>", prefix);
            h78USBDPWRT(line, len);
            h78USBDPLN("<");
            return (h78SUCCESS);
        }
    }
    h78USBDPLN("TIMEOUT><");

    return (h78ERR_TIMED_OUT);
}

/**
 *  @fn
 *
 *  1行分のレスポンスを取得する
 *
 *  @param(limit)       [in] 取得する期限[mS]
 *  @param(limit)       [out] 取得したデータの格納先
 *  @param(limit)       [in] lineのサイズ[Bytes]
 *  @return             0:タイムアウト時、1～:成功時(取得したバイト数)
 *  @detail
 */
int HL7800::getLine(uint32_t limit, char *line, int size) {
    char *top = line;
    while (millis() < limit) {
        int c = 0;
        if ((c = h78SERIAL.read()) < 0)
            continue;
        if (--size > 0)
            *line++ = (char)c;
        if (c == '\n') {
#ifdef DEBUG_USB
            h78USBDP("<getLine() return: %d,\"", (int)(line - top));
            for (int i = 0; i < (int)(line - top); i++)
                h78USBDPC(top[i]);
            h78USBDPLN("\"");
#endif // DEBUG_USB
            return ((int)(line - top));  // reach end of line
        }
    }
    h78USBDPLN("<getLine() T/O\n");

    return (0);     // timeout
}

// End of hl7800_private.cpp
