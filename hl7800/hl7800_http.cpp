/*
 *  hl7800_http.cpp
 *
 *  Control library for HL7800 (HTTP function)
 *
 *  R0  2020/02/16 (A.D)
 *  R1  2020/08/03 (A.D)
 *  R2  2020/10/11 (A.D)
 *  R2a 2020/11/14 (A.D)
 *  R3  2021/06/21 (A.D) fix getBody() when empty body
 *  R5  2021/08/16 (A.D) add setRootCA() and getLastHttpStatusCode(), so we support "https:" from now on.
 *
 *  Copyright(c) 2020-2021 TABrain Inc. All rights reserved.
 */

#include "hl7800.h"

/**
 *  @fn     doHttpGet
 *
 *  HTTP/GETを実行する
 *
 *  @param(url)         [in] URL
 *  @param(header)      [in] リクエストヘッダの文字列
 *  @param(response)    [out] レスポンスボディの格納先
 *  @param(nbytes)      [in/out] responseのサイズ／実際に取得したレスポンスのサイズ[Bytes]
 *  @return             0:成功時、0～:エラー時(エラーコード)、～0:エラー時(HTTPステータスをマイナスにした値)
 *  @detail             R1までは、"https:""はサポートしていない
 *
 */
int  HL7800::doHttpGet(char *url, char *header, char *response, int *nbytes) {
    h78USBDPLN(">doHttpGet(\"%s\",\"%s\",resp,%d)", url, ((header != NULL) ? header : "-"), *nbytes);

    char host[h78MAX_HOST_LENGTH+1], path[h78MAX_PATH_SIZE+1];
    int  stat, port = 0, useSSL = 0;

    // split url into host, port and path
    if (splitUrl(url, host, &port, path, &useSSL) != 0) {
        // url is illegal format
        return (h78ERR_BAD_PARAM);    // Bad parameters
    }

    // URL and port
    h78SENDFLN("AT+KHTTPCFG=1,\"%s\",%d,%d", host, port, (useSSL ? 2 : 0));
    if ((stat = getSessionId(h78TIMEOUT_LOCAL, "+KHTTPCFG:", &_httpSessionId)) != 0) {
        // ERROR or TIMEOUT
        h78USBDPLN("+>KHTTPCFG NG");
        return (h78ERR_HTTP_SESSIONID);
    }
    h78USBDPLN("+>KHTTPCFG OK: %d", _httpSessionId);

    // Wait until HTTP is ready
    if ((stat = waitUntilReady(h78TIMEOUT_HTTP_READY, "+KHTTP_IND:", _httpSessionId)) != 0) {
        // ERROR or TIMEOUT
        h78USBDPLN("+>KHTTP_IND *,1 Not Found");
        stat = h78ERR_HTTP_READY;
        goto WindUp;
    }
    h78USBDPLN("+>KHTTP_IND OK");

    // Headerを送る
    if (header != NULL) {
        h78SENDFLN("AT+KHTTPHEADER=%d", _httpSessionId);
        delay(200);
        if (waitUntilCONNECT(h78TIMEOUT_GET) == 0) {
            int headerSize = strlen(header);
            h78SEND((uint8_t *)header, headerSize);
            delay(200);
            h78SENDF(h78END_PATTERN);
            int len = *nbytes - 1;
            if ((stat = getResponse(h78TIMEOUT_GET, response, &len)) != 0) {
                // ERROR or TIMEOUT
                h78USBDPLN("+>KHTTPHEADER NG");
                stat = h78ERR_HTTP_HEADER;
                goto WindUp;
            }
        }
        else {
            // ERROR or TIMEOUT
            h78USBDPLN("+>KHTTPHEADER CONNECT NG");
            stat = h78ERR_HTTP_CONNECT;
            goto WindUp;
        }
        h78USBDPLN("+>KHTTPHEADER OK");
    }

    discardResponse(30);  // Discard gomi @@

    // GETメソッドを送出し、レスポンスを取得する
    h78SENDFLN("AT+KHTTPGET=%d,\"%s\"", _httpSessionId, path);
    delay(200);
    if (waitUntilCONNECT(h78TIMEOUT_GET) == h78SUCCESS) {
        // Get response header
        int httpStatusCode, contentLength;
        if (parseHeader(&httpStatusCode, &contentLength) != h78SUCCESS) {
            // Error or timeout
            stat = h78ERR_HTTP_HEADER_RES;
            goto WindUp;
        }
        _lastHttpStatusCode = httpStatusCode;
        // Get response body
        int len = *nbytes;    // set buffer size
        if ((stat = getBody(response, &len, contentLength, h78TIMEOUT_BODY)) != h78SUCCESS) {
            // Error or timeout
            stat = h78ERR_HTTP_BODY_RES;
            goto WindUp;  // Error(Bad body) or timeout
        }
        *nbytes = len;
        h78USBDPLN("+>KHTTPGET OK");
        // Check HTTP status
        if (100 <= httpStatusCode && httpStatusCode < 400) {
            // HTTP 0 (1xx, 2xx or 3xx)
            stat = h78SUCCESS;
        }
        else {
            // HTTP error (4xx or 5xx) - negative value
            stat = - httpStatusCode;
        }
    }
    else {
        // Bad url or timeout
        stat = h78ERR_HTTP_GET;
    }

  WindUp:
    // Wind up HTTP/GET session
    closeHttpSession();    // Close session and clear _httpSessionId

    return (stat);
}

/**
 *  @fn     doHttpPost
 *
 *  HTTP/POSTを実行する
 *
 *  @param(url)         [in] URL
 *  @param(header)      [in] リクエストヘッダの文字列('\0'で終端すること、省略時はNULLを指定する）
 *  @param(body)        [in] リクエストボディ(バイナリデータも可、ただしEODパターンを含まないこと)
 *  @param(bodySize)    [in] リクエストボディのサイズ[Bytes]
 *  @param(response)    [out] レスポンスボディの格納先
 *  @param(nbytes)      [in/out] responseのサイズ／実際に取得したレスポンスのサイズ[Bytes]
 *  @return             0:成功時、0～:エラー時(エラーコード)、～0:エラー時(HTTPステータスをマイナスにした値)
 *  @detail             R1までは、"https:""はサポートしていない
 *
 */
int HL7800::doHttpPost(char *url, char *header, void *body, int bodySize, char *response, int *responseSize) {
    h78USBDPLN(">doHttpPost(\"%s\",\"%s\",\"%s\",%d,-,%d", url, ((header != NULL) ? header : "-"), body,  bodySize, *responseSize);

    char host[h78MAX_HOST_LENGTH+1], path[h78MAX_PATH_SIZE+1];
    int  port, stat, useSSL = 0;

    // split url into host, port and path
    if (splitUrl(url, host, &port, path, &useSSL) != 0) {
        // url is illegal format
        return (h78ERR_BAD_PARAM);    // Bad parameters
    }

    // URL and port
    h78SENDFLN("AT+KHTTPCFG=1,\"%s\",%d,%d", host, port, (useSSL ? 2 : 0));
    if ((stat = getSessionId(h78TIMEOUT_LOCAL, "+KHTTPCFG:", &_httpSessionId)) != 0) {
        h78USBDPLN("+>KHTTPCFG NG");
        return (h78ERR_HTTP_SESSIONID);
    }
    h78USBDPLN("+>KHTTPCFG OK");

    // Wait until HTTP is ready
    if ((stat = waitUntilReady(h78TIMEOUT_HTTP_READY, "+KHTTP_IND:", _httpSessionId)) != 0) {
        h78USBDPLN("+>KHTTP_IND *,1 Not Found");
        closeHttpSession();    // Close session and clear _httpSessionId
        return (h78ERR_HTTP_READY);
    }
    h78USBDPLN("+>KHTTP_IND OK");

    // Header
    stat = 1;
    h78SENDFLN("AT+KHTTPHEADER=%d", _httpSessionId);
    if (waitUntilCONNECT(h78TIMEOUT_POST) == 0) {
        h78SENDFLN("Content-length:%d\r\n", bodySize);
        if (header != NULL) {
            // Send specified header
            h78SENDF(header);
            int headerSize = strlen(header);
            char lastChar = header[headerSize-1];
            if (lastChar != '\n' && lastChar != '\r')
                h78SENDF("\r\n");    // supplement a newline at the end
        }
        h78SENDF(h78END_PATTERN);
        delay(500);
        int len = *responseSize - 1;
        if ((stat = getResponse(h78TIMEOUT_POST, response, &len)) != 0) {
            h78USBDPLN("+>KHTTPHEADER NG");
            closeHttpSession();    // Close session and clear _httpSessionId
            return (h78ERR_HTTP_HEADER);
        }
    }
    else {
        h78USBDPLN("+>KHTTPHEADER CONNECT NG");
        closeHttpSession();    // Close session and clear _httpSessionId
        return (h78ERR_HTTP_CONNECT);
    }
    h78USBDPLN("+>KHTTPHEADER OK");

    discardResponse(30);  // Discard gomi

    // Do POST
    h78SENDFLN("AT+KHTTPPOST=%d,,\"%s\"", _httpSessionId, path);

    if (waitUntilCONNECT(h78TIMEOUT_POST) == 0) {
#if defined(_USE_HW_FLOW_CONTROL_)
        // Send body at a once
        h78SEND((uint8_t *)body, bodySize);
#else
        // Send chunkSize bytes at a time to prevent the UART from overflowing
        const int chunkSize = 2048;
        while (bodySize > 0) {
            int bytes = (chunkSize > bodySize) ? bodySize : chunkSize;
            h78SEND((uint8_t *)body, bytes);
            body += bytes;
            bodySize -= bytes;
            delay(200);
        }
#endif
        h78SENDF(h78END_PATTERN);
        // Get response header
        int httpStatusCode, contentLength;
        if (parseHeader(&httpStatusCode, &contentLength) != 0) {
            closeHttpSession();    // Close session and clear _httpSessionId
            return (h78ERR_HTTP_HEADER_RES);
        }

        // Get response body
        int len = h78MAX_RESPONSE_LENGTH;    // max length is compatible v1
        if ((stat = getBody(response, &len, contentLength, h78TIMEOUT_BODY)) != 0) {
            closeHttpSession();    // Close session and clear _httpSessionId
            return (h78ERR_HTTP_BODY_RES);
        }
        *responseSize = len;
        h78USBDPLN("+>KHTTPPOST OK: %d", httpStatusCode);
        // Check HTTP status
        if (100 <= httpStatusCode && httpStatusCode < 400) {
            // HTTP 0 (1xx, 2xx or 3xx)
            stat = h78SUCCESS;
        }
        else {
            // HTTP error (4xx or 5xx) - negative value
            stat = - httpStatusCode;
        }
    }
    else {
        // Bad url or timeout
        stat = h78ERR_HTTP_POST;
    }

    // Wind up HTTP/POST session
    closeHttpSession();    // Close session and clear _httpSessionId

    h78USBDPLN("<doHttpPost(): %d", stat);
    return (stat);
}

/**
 *  @fn     seRootCA
 *
 *  RootCA証明書を登録する
 *
 *  @param(rootCA)      [in] 登録するRooTCA証明書の文字列
 *  @return             0:成功時、0～:エラー時(エラーコード)
 *  @detail             doHttpGet()/doHttpPost()でhttpsを使用する証明書を登録する
 *
 */
int HL7800::setRootCA(char *rootCA) {
    if (! rootCA)
        return (h78ERR_HTTP_BAD_CA);  // root CA is null
    int length = strlen(rootCA);
    if (length > 4096)
        return (h78ERR_HTTP_BAD_CA);  // too long CA

    // Send KCERTSTORE command with root CA
    int stat = 1;
    h78SENDFLN("AT+KCERTSTORE=0,%d", length);
    if (waitUntilCONNECT(h78TIMEOUT_LOCAL) == 0) {
        h78SEND((uint8_t *)rootCA, length);
        delay(100);
        char response[30];
        int len = sizeof(response) - 1;
        if ((stat = getResponse(h78TIMEOUT_LOCAL, response, &len)) != 0) {
            response[len] = '\0';
            h78USBDPLN("+>KCERTSTORE NG: %s", response);
            return (h78ERR_HTTP_ERR_CA);
        }
    }
    else {
        h78USBDPLN("+>KCERTSTORE CONNECT NG");
        return (h78ERR_HTTP_ERR_CA);
    }
    h78USBDPLN("+>KCERTSTORE OK");

    return (h78SUCCESS);
}

/**
 *  @fn     splitUrl
 *
 *  urlを解析して、分解する
 *
 *  @param(url)         [in] 解析・分解するURL
 *                          The format patterns of url are one of following:
 *                            - http://host:port/path
 *                            - http://host:port
 *                            - http://host/path
 *                            - http://host
 *                            - https://host:port/path
 *                            - https://host:port
 *                            - https://host/path
 *                            - https://host
 *  @param(host)        [out] 抽出したホスト
 *  @param(port)        [out] 抽出したポート番号
 *  @param(path)        [out] 抽出したパス
 *  @param(useSSL)      [out] SSLを使用するか(1)/しないか(0)
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::splitUrl(char *url, char *host, int *port, char *path, int *useSSL) {
    char *hp = host, *pp = path;
    int portNum;
    // Check protocol
    if (! strncmp(url, "https://", 8)) {
        portNum = 443;  // https default port number
        url += 8;
        *useSSL = 1;
    }
    else if (! strncmp(url, "http://", 7)) {
        portNum = 80;  // http default port number
        url += 7;
        *useSSL = 0;
    }
    else {
        h78USBDPLN(">splitUrl() error: unknown protocol");
        return (-1);    // unknown protocol
    }

    int split = 0;  // url has split(1) or not(0)
    for (char *p = url; *p != '\0'; p++) {
        if (*p == ':') {
            int len = 0;
            for (char *q = url; q < p; q++) {
                if (++len > h78MAX_HOST_LENGTH) {
                    h78USBDPLN(">splitUrl() too long host(max %d)", h78MAX_HOST_LENGTH);
                    return (-3);
                }
                *host++ = *q;
            }
            *host = '\0';
            p++;    // skip ':'
            portNum = atoi(p);
            while (*p != '\0' && isdigit(*p))
                p++;  // skip port number digits
            if (*p == '\0')
                strcpy(path, "/");
            else
                strcpy(path, p);
            split = 1;  // OK
            break;
        }
        else if (*p == '/') {
            int len = 0;
            for (char *q = url; q < p; q++) {
                if (++len > h78MAX_HOST_LENGTH) {
                    h78USBDPLN(">splitUrl() too long host(max %d)", h78MAX_HOST_LENGTH);
                    return (-3);
                }
                *host++ = *q;
            }
            *host = '\0';
            strcpy(path, p);
            split = 1;
            break;    // OK
        }
    }

    if (! split) {
        // no path specified
        strcpy(host, url);
        strcpy(path, "/");
    }

    *port = portNum;

    h78USBDPLN(">splitUrl() return: \"%s\",%d,\"%s\",%d", hp, portNum, pp, *useSSL);

    return (h78SUCCESS);
}

/**
 *  @fn     getBody
 *
 *  レスポンスボディを取得・解析する
 *
 *  @param(resp)        [in/out] 取得したボディの格納先
 *  @param(size)        [in/out] 取得したボディのサイズ[Bytes]
 *  @param(contentLength) [in] レスポンスヘッダで指定されたボディサイズ[Bytes]
 *  @param(timeout)     [in] タイムアウト時間[mS]
 *  @return             0:成功時、0以外:エラー時(エラーコード)
 *  @detail             EODを検出する処理で、高速化の余地あり
 */
int  HL7800::getBody(char *resp, int *size, int contentLength, uint32_t timeout) {
    uint32_t limit = millis() + timeout;
    char *body = resp;
    int  length = 0;        //
    int  readBytes;         // The number of bytes that it must retrieve from UART1 buffer

    h78USBDPLN(">getBody(-,%d,%d,%u)", *size, contentLength, timeout);

    // There is HL7800's firmware bug, so return immediately when empty body
    if (contentLength == 0) {
        *size = 0;
        discardResponse(100);
        h78USBDPLN("body=>\"\",0<");
        return (h78SUCCESS);
    }

    if (contentLength < 0) {
        readBytes = *size;          // I don't know the size, so I read only the size of the buffer @@@
    }
    else {
        readBytes = contentLength + strlen(h78END_PATTERN);
        if (contentLength < *size)
            *size = contentLength;  // set result size to store into buffer
    }

    h78USBDP("RESPONSE>>");
    while ((millis() <= limit) && (readBytes > 0)) {
        int c;
        if ((c = h78SERIAL.read()) <= 0)
            continue;   // sometimes ignore it because it contains zero

        readBytes--;
        h78USBDPC((char)c);

        if (length < *size) {
            *resp++ = (char)(c & 0xff);
            length++;
        }
    }
    h78USBDPLN("<<%d", *size);
    *resp = '\0';
    *size = length;

    // remove EOD from response
    char *p = strstr(body, h78END_PATTERN);
    if (p != NULL) {
        *p = '\0';
        *size = strlen(body);

    }

    if (millis() >= limit) {
        h78USBDPLN("*>T.O");
        // hangUp();     // I want to let you actually hang up, but can't do so.. ("+++" command is not stable)
        return (h78ERR_TIMED_OUT);
    }

    h78USBDPLN("body=>\"%s\",%d<", body, length);

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  レスポンスヘッダを取得・解析する
 *
 *  @param(httpStatusCode)  [out] HTTPステータスコード
 *  @param(contentLength)   [out] ボディのサイズ[Bytes] または -1(サイズ不明)
 *  @return                 0:成功時、0以外:エラー時
 *  @detail                 ボディとの境界を検出するロジックは改良の余地あり
 *                          ヘッダの終わりは空行であることを仮定する(2020/7～)
 */
int  HL7800::parseHeader(int *httpStatusCode, int *contentLength) {
    uint32_t limit = millis() + h78TIMEOUT_HEADER;
    int code = *httpStatusCode = -1;    // "HTTP/1.x 999 message.."
    int length = *contentLength =  -1;  // "Content-Length" property value

    // skip headers and get http status code & content length
    while (true) {
        // Get a line
        char line[200];
        int len;
        h78SERIAL.setTimeout(limit - millis());
        if ((len = h78SERIAL.readBytesUntil('\n', line, sizeof(line)-2)) == 0)
            return (h78ERR_HTTP_HEADER_RES);   // 予期しないエラー

        line[len] = '\0';
        h78USBDPLN("line=[%s]", line);
        delayMicroseconds(100);      // @ why need this line ?
        if (line[0] == '\r' || len == 1) {
            // empty line(= end of header) found
            h78USBDPLN("*>header end");
            break;
        }

/**
        // 空行がない時の暫定処理（ただし、不確実）
        char next = h78SERIAL.peek();
        if (! isalpha(next)) {
            // 行の先頭が英字以外で始まる行はヘッダではない
            h78USBDPLN("*>header end(no empty line)");
            break;
        }
**/
        if (code < 0 && ! strncmp(line, "HTTP/", 5)) {
            int offset = 8;    // at least 8 bytes
            while (isspace(line[offset]))
                offset++;
            *httpStatusCode = code = atoi(line + offset);
            h78USBDPLN(">HTTP Status Code=%d", code);
        }
        else if (length < 0 && (! strncmp(line, "Content-Length:", 15) || ! strncmp(line, "content-length:", 15))) {
            int offset = 15;  // at least 15 bytes
            while (isspace(line[offset]))
                offset++;
            *contentLength = length = atoi(line + offset);
            h78USBDPLN(">Content-Length=%d", length);
        }
    }

    h78USBDPLN(">parseHeader() done: %d,%d", code, length);

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  アクティブなHTTPセッションをクローズして解放する
 *
 *  @param              なし
 *  @return             なし
 *  @detail             クローズ処理に時間が掛かる場合がある
 *                      各ATコマンドのレスポンスをチェックしても無意味なので、無視する
 */
void HL7800::closeHttpSession(void) {
    if (_httpSessionId > 0) {
        // Close and delete session
        h78SENDFLN("AT+KHTTPCLOSE=%d", _httpSessionId);
        discardResponse(h78WAITTIME_LOCAL);
        h78SENDFLN("AT+KHTTPDEL=%d", _httpSessionId);
        discardResponse(h78WAITTIME_LOCAL);
        _httpSessionId = 0;    // Clear _httpSessionId
    }

    h78USBDPLN(">closeHttpSession() done");
}

/**
 *  @fn
 *
 *  データモードを抜けて、コマンドモードに戻る
 *
 *  @param              なし
 *  @return             なし
 *  @detail             うまく動作しない..
 */
void HL7800::hangUp(void) {
    //-- Hang up http/https conection -- not stable
    delay(100);
    h78SENDF("+++");
    delay(100);
    h78USBDPLN("*>HANGUP!!");
}

// End of hl7800_http.cpp
