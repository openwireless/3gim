/*
 *  hl7800.cpp
 *
 *  Control library for HL7800 (Control functions)
 *
 *  R0  2020/02/16 (A.D)
 *  R1  2020/06/21 (A.D)
 *  R1a 2020/11/19 (A.D)  h78SERIAL.flush()を各関数の出口で呼び出すよう修正
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#include "hl7800.h"

/*
 *  Local Symbols
 */
#define MAX_SESSION_ID          5           // 最大のセッションID
#define BOOTING_TIME            7000        // HL7800のブートに要する時間[mS]
#define MAX_RETRY_CGATT         5           // AT+CGATTコマンドをリトライする回数

/**
 *  @fn
 *
 *  hl7800の電源をOnにして、初期化に必要なATコマンドを実行し、ライブラリを使用可能にする
 *
 *  @return             0:成功時、0以外:エラー時
 *  @detail             HL7800の起動のため、BOOTING_TIME時間だけ待つ
 */
int HL7800::begin(void) {
#ifdef DUMMY_TEST
    _initialized = true;
    return (h78SUCCESS)
#endif

    if (_initialized)
        return (h78ERR_ALREADY_INITIALIZED);

    // Set TCP timeout values
    _timeoutTcpConnect = h78TIMEOUT_CONNECT;
    _timeoutTcpWrite = h78TIMEOUT_WRITE;

    // Set control pins
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW);       // Deactive RESET_IN_N(HIGH)
    pinMode(_wakeUpPin, OUTPUT);
    digitalWrite(_wakeUpPin, LOW);      // Deactive WAKE_UP(LOW)
    pinMode(_powerOnPin, OUTPUT);
    digitalWrite(_powerOnPin, HIGH);    // Active PWR_ON_N(LOW)
    delay(200);
    digitalWrite(_powerOnPin, LOW);    // Deactive PWR_ON_N(HIGH)

    h78SERIAL.begin(h78BAUDRATE);

    delay(BOOTING_TIME);   //@@ 時間調整が必要

    // 初期化のためのATコマンドを送る
    h78SENDFLN("ATZ");                            // 1回目のATコマンドの実行は失敗するので、当たり障りのないコマンドを実行しておく
    discardResponse(h78WAITTIME_LOCAL);
    h78SENDFLN("ATE1");                            // Echo back on
    discardResponse(h78WAITTIME_LOCAL);
    h78SENDFLN("AT+KSLEEP=2");                     // Don't sleep
    discardResponse(h78WAITTIME_LOCAL);
    h78SENDFLN("AT+KPATTERN=\"%s\"", h78END_PATTERN);     // set SHORT eof pattern string
    discardResponse(h78WAITTIME_LOCAL);

/**
    // 残っているセッションIDを削除しておく（どうやらNVMに残るらしい、、）
    for (int n = 1; n <= MAX_SESSION_ID; n++) {
        char resp[20];
        int respSize = sizeof(resp) - 1;
        h78SENDFLN("AT+KHTTPDEL=%d", n);
        getResponse(h78WAITTIME_LOCAL, resp, &respSize);
    }
    for (int n = 1; n <= MAX_SESSION_ID; n++) {
        char resp[20];
        int respSize = sizeof(resp) - 1;
        h78SENDFLN("AT+KUDPDEL=%d", n);
        getResponse(h78WAITTIME_LOCAL, resp, &respSize);
    }
    for (int n = 1; n <= MAX_SESSION_ID; n++) {
        char resp[20];
        int respSize = sizeof(resp) - 1;
        h78SENDFLN("AT+KTCPDEL=%d", n);
        getResponse(h78WAITTIME_LOCAL, resp, &respSize);
    }
**/

    // Private変数を初期化しておく
    _tcpSessionId = 0;
    _udpSessionId = 0;
    _httpSessionId = 0;
    _initialized = true;

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  hl7800ライブラリを終了する
 *
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::end(void){
#ifdef DUMMY_TEST
    _initialized = false;
    return (h78SUCCESS);
#endif

    if (! _initialized)
        return (h78ERR_NOT_YET_INITIALIZED);    // NG: not initialized yet.

    h78SERIAL.flush();
    h78SERIAL.end();

    _initialized = false;

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  IMEIを取得する
 *
 *  @param(imei)        [out] 取得したIMEI
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::getIMEI(char imei[]) {
#ifdef DUMMY_TEST
    strcpy(imei, "123456789012345");
    return (h78SUCCESS);
#endif

    // Clear USRT buffer
    while (h78SERIAL.available() > 0)
        h78SERIAL.read();

    int  stat = 0;
    char response[50];
    int len = sizeof(response) - 1;
    h78SENDFLN("AT+CGSN");
    if ((stat = getResponse(h78WAITTIME_LOCAL, response, &len)) == 0 && len > (h78IMEI_SIZE + 9)) {
        strncpy(imei, response + 11, h78IMEI_SIZE);
        imei[h78IMEI_SIZE] = '\0';
        stat = h78SUCCESS;
    }
    else
        stat = h78ERR_CANOT_GET_IMEI;

    h78SERIAL.flush();      // ゴミを読み捨てる(@R1a)

    return (stat);
}

/**
 *  @fn
 *
 *  現在の日時をLTE網から取得する
 *
 *  @param(datetime)    [out] 取得した日時(YYYY/MM/DD hh:mm:ss形式、24時間形式)
 *  @return             0:成功時、0以外:エラー時
 *  @detail             リトライはしない
 */
int HL7800::getDateTime(char datetime[]) {
#ifdef DUMMY_TEST
    strcpy(datetime, "2019/05/05 12:00:00");
    return (h78SUCCESS);     // OK
#endif

    h78SENDFLN("AT+CCLK?");
    char response[100];
    int stat, len = sizeof(response) - 1;
    if ((stat = getResponse(h78WAITTIME_LOCAL, response, &len)) == 0) {
        response[len] = '\0';
        h78USBDPLN("RES>%s<RES", response);
        if ((stat = parseCCLK(response, datetime)) == 0)
            stat = h78SUCCESS;
        else
            stat = h78ERR_INTERNAL_ERROR;
    }
    else
        stat = h78ERR_CANOT_GET_DATETIME;

    h78SERIAL.flush();      // ゴミを読み捨てる(@R1a)

    return (stat);
}

/**
 *  @fn
 *
 *  現在の日時をLTE網から取得する
 *
 *  @param(datetime)    [out] 取得した日時(DATE_TIME形式)
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::getDateTime(DATE_TIME *datetime) {
    h78SENDFLN("AT+CCLK?");
    char response[100], dt[20];
    int stat, len = sizeof(response) - 1;
    if ((stat = getResponse(h78WAITTIME_LOCAL, response, &len)) == 0) {
        response[len] = '\0';
        h78USBDPLN("RES>%s<RES", response);
        if ((stat = parseCCLK(response, dt)) == 0)
            stat = h78SUCCESS;
        else
            stat = h78ERR_INTERNAL_ERROR;
    }
    else
        stat = h78ERR_CANOT_GET_DATETIME;

    if (stat == h78SUCCESS) {
        // dt[] = "YYYY/MM/DD hh:mm:ss"
        datetime->day = atoi(dt+8);
        datetime->month = atoi(dt+5);
        datetime->year = atoi(dt+2);    // 下2桁を扱う
        datetime->hours = atoi(dt+11);
        datetime->minutes = atoi(dt+14);
        datetime->seconds = atoi(dt+17);
    }

    h78SERIAL.flush();      // ゴミを読み捨てる(@R1a)

    return (stat);
}

/**
 *  @fn
 *
 *  現在の電波強度を取得する
 *
 *  @param(rssi)        [out] 取得した電波強度RSSI(単位はdBm)
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::getRSSI(int *rssi) {
#ifdef DUMMY_TEST
    rssi = -88;
    return (h78SUCCESS);     // OK
#endif

    int  stat = 0;
    char response[100];
    int len = sizeof(response) - 1;
    h78SENDFLN("AT+CSQ");  // Signal Quality
    if ((stat = getResponse(h78WAITTIME_LOCAL, response, &len)) == 0) {
        h78USBDPLN("RES>%s<RES", response);
        char sq[5];
        int i, offset = 15;
        for (i = 0; i < 3 && response[i+offset] != ','; i++)
            sq[i] = response[i + offset];
        sq[i] = '\0';
        if (atoi(sq) == 99) {
            // Can't get signal quarity
            stat = h78ERR_CANOT_GET_RSSI;
        }
        else {
            *rssi = (2 * atoi(sq)) - 113;    // Convert signal quality to rssi
            stat = h78SUCCESS;
        }
    }
    else
        stat = h78ERR_INTERNAL_ERROR;

    h78SERIAL.flush();      // ゴミを読み捨てる(@R1a)

    return (stat);
}

/**
 *  @fn
 *
 *  通信サービスの可否を取得する
 *
 *  @param(state)       [out] 取得した通信サービス状態(0:通信サービス利用不可/1:通信サービス利用可)
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::getService(int *state) {
#ifdef DUMMY_TEST
    state = 1;
    return (h78SUCCESS);     // OK
#endif

    int stat = 0;
    h78SENDFLN("AT+CGATT?");  // Signal Quality
    h78USBDPLN("AT+CGATT?");
    if (parseCGATT(state) == h78SUCCESS) {
        stat = h78SUCCESS;
    }
    else
        stat = h78ERR_INTERNAL_ERROR;

    h78SERIAL.flush();      // ゴミを読み捨てる(@R1a)

    return (stat);
}

/**
 *  @fn
 *
 *  プロファイル(APN)を設定する
 *
 *  @param(apn)         [in] APN
 *  @param(user)        [in] User
 *  @param(password)    [in] Password
 *  @return             0:成功時、0以外:エラー時
 *  @detail             実際にLTE網に接続するため、やや時間が掛かる
 *                      AT+CGATTは失敗する確率が高いので、MAX_RETRY_CGATT回だけリトライする
 */
int HL7800::setProfile(char *apn, char *user, char *password) {
#ifdef DUMMY_TEST
    return (h78SUCCESS);
#endif
    h78USBDPLN(">setProfile(%s,%s,%s)", apn, user, password);

    char resp[20];
    h78SENDFLN("AT+KCNXCFG=1,\"GPRS\",\"%s\",\"%s\",\"%s\"", apn, user, password);
    int stat, respSize = sizeof(resp) - 1;
    if ((stat = getResponse(h78WAITTIME_LOCAL, resp, &respSize)) != h78SUCCESS) {
        h78USBDPLN("AT+KCNXCFG error: %d", stat);
        return (h78ERR_CANOT_SET_PROFILE);
    }
    h78USBDPLN("AT+KCNXCFG OK");

    h78SENDFLN("AT+KCNXPROFILE=1");  // <cnx cnf> is always 1.
    respSize = sizeof(resp) - 1;
    if ((stat = getResponse(h78WAITTIME_LOCAL, resp, &respSize)) != h78SUCCESS) {
        h78USBDPLN("AT+KCNXPROFILE error: %d", stat);
        return (h78ERR_CANOT_SET_PROFILE);
    }
    h78USBDPLN("AT+KCNXPROFILE OK");

    int retryCount = MAX_RETRY_CGATT;
    while (retryCount-- > 0) {
        h78SENDFLN("AT+CGATT=1");        // Attach lte network
        respSize = sizeof(resp) - 1;
        if ((stat = getResponse(h78TIMEOUT_CGATT, resp, &respSize)) == h78SUCCESS)
            break;
    }
    if (retryCount == 0) {
        h78USBDPLN("AT+CGATT error: %d", stat);
        return (h78ERR_CANOT_ATTACH_LTE);
    }
    h78USBDPLN("AT+CGATT OK");

    h78USBDPLN("<setProfile() OK");
    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  HL7800の電源を切る
 *
 *  @param(mode)        [in] 電源をOffにするモード(POFF_POWER_OFF/POFF_HIBANATE)
 *  @return             0:成功時、0以外:エラー時
 *  @detail             POFF_IMMEDIATELYが指定されたときは、RESET_IN_Nピンを使って強制的にHL7800をシャットダウンする
 */
int HL7800::powerOff(POFF_MODE mode) {
    switch (mode) {
      case POFF_NORMALY:
        h78SENDFLN("AT+CPOF");
        {
            char response[100];
            int len = sizeof(response) - 1;
            if (getResponse(h78WAITTIME_LOCAL, response, &len) == 0)
                return (h78ERR_INTERNAL_ERROR);
        }
        digitalWrite(_resetPin, HIGH);
        break;
      case POFF_IMMEDIATELY:
        // Force turn off by RESET_IN_N pin
        digitalWrite(_resetPin, HIGH);
        delay(100);
        break;
      default:
        return (h78ERR_BAD_PARAM);
    }

    digitalWrite(_wakeUpPin, LOW);
    digitalWrite(_powerOnPin, LOW);

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  HL7800の電源を入れる
 *
 *  @return             0:成功時、0以外:エラー時
 *  @detail             WAKE_UPピンでHL7800を起こす
 */
int HL7800::powerOn(void) {
    // PWR_ON_NをLOWにする
    digitalWrite(_powerOnPin, HIGH);
    delay(200);

    // WAKE_UPをHIGHにする
    digitalWrite(_wakeUpPin, HIGH);
    delay(200);

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  HL7800の電源が入っているかをチェックする
 *
 *  @return             true:On、false:Off
 *  @detail             HL7800のVGPIOピンの電圧で判断する
 */
boolean HL7800::isPowerOn(void) {
    int mV = analogRead(_1v8pin) * 3.226;
    return (mV > 1700);
}

/**
 *  @fn
 *
 *  HL7800をリセットする
 *
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::reset(void) {
    // Reset HL7800 - not support yet.
    // Force turn off by RESET_IN_N pin
    digitalWrite(_resetPin, HIGH);
    delay(100);
    digitalWrite(_resetPin, LOW);
    delay(BOOTING_TIME);

    return (h78SUCCESS);
}

/**
 *  @fn
 *
 *  ATコマンドを実行する
 *
 *  @return             0:成功時、0以外:エラー時
 *  @detail
 */
int HL7800::doAT(char *at) {
    int  stat = 0;
    // Reset HL7800 - not support yet.
    return stat;
}

/**
 *  @fn
 *
 *  コマンドのレスポンスを取得・解析する
 *
 *  @param(timeout)     [in] タイムアウト時間[mS]
 *  @param(resp)        [out] 取得したレスポンスの格納先
 *  @param(size)        [in/out] respのサイズ／取得したレスポンスのサイズ[Bytes]
 *  @return             0:成功時(OK)、0以外:エラー時(エラーコード)
 *  @detail
 */
int HL7800::getResponse(uint32_t timeout, char *resp, int *size) {
#ifdef DEBUG_USB
    h78USBDPLN(">getResponse(%d,-,%d)", timeout, *size);
    char *top = resp;
#endif

    int length = 0;
    uint32_t limit = millis() + timeout;
    while (millis() < limit) {
        char lastline[10];
        int len = *size - length;
        if (len < sizeof(lastline) - 1) {
            h78USBDP("++else1++BEGIN++");
            len = sizeof(lastline) - 1;
            resp =lastline;
            if ((len = getLine(limit, resp, len)) == 0)
                break;    // Timed out
            h78USBDPLN("++else1++END++ %d", len);
        }
        else {
            h78USBDP("++else2++BEGIN++");
            if ((len = getLine(limit, resp, len)) == 0)
                break;    // Timed out
            length += len;
            h78USBDPLN("++else2++END++ %d", len);
        }

        if (len == 4 && ! strncmp(resp, "OK\r\n", 4)) {
            *size = length;
            h78USBDPLN("OK>>\"%s\",%d<<", top, *size);
            return (h78SUCCESS);
        }
        else if (len == 7 && ! strncmp(resp, "ERROR\r\n", 7)) {
            *size = length;
            h78USBDPLN("ERROR>>\"%s\",%d<<", top, *size);
            return (h78ERR_ERROR);
        }
        resp += len;
    }
    h78USBDPLN("T/O>>%s,%d<<", top, length);

    return (h78ERR_TIMED_OUT);
}

// End of hl7800.cpp
