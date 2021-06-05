/*
 *  hl7800.h
 *
 *  Control library for HL7800(LTE-M/NB1 module)
 *
 *  R0  2020/02/16 (A.D)
 *  R1  2020/08/03 (A.D)
 *  R2  2020/10/11 (A.D)
 *  R3  2021/05/05 (A.D) change for mgim(V4.1)
 *
 *  Copyright(c) 2020 TABrain Inc. All rights reserved.
 */

#ifndef _hl7800_h_
#define  _hl7800_h_

#include <Arduino.h>
#include <mgim.h>

//-- DEBUG CONFIGURATION --
//#define DUMMY_TEST
//#define DEBUG_USB                             // When this symbol is defined, you can debug by connecting a PC to USB

// Symbols
#define h78SERIAL                   Serial      // Serial port with HL7800
#define h78BAUDRATE                 115200UL    // Baudrate between MCU and HL7800
#define h78END_PATTERN              "@EOD@"     // Replace default end pattern (The pattern is hard to be included in the data)
  // timeout
#define h78WAITTIME_LOCAL           500         // Wait time for local command
#define h78TIMEOUT_LOCAL            3000        // Timeout of local command
#define h78TIMEOUT_CGATT            60000       // Timeout of AT+CGATT command
#define h78TIMEOUT_GET              30000       // Timeout of http/get response[mS] - because of overhead to discard a long response
#define h78TIMEOUT_POST             30000       // Timeout of http/post response[mS]
#define h78TIMEOUT_BODY             30000       // Timeout of reading response body fully [mS]
#define h78TIMEOUT_HEADER           30000       // Timeout of reading response header fully [mS]
#define h78TIMEOUT_HTTP_READY       10000       // Timeout of http/https ready [mS]
#define h78TIMEOUT_CONNECT          60000       // Timeout of tcp connect [mS]
#define h78TIMEOUT_WRITE            60000       // Timeout of tcp write [mS]
#define h78TIMEOUT_WRITE_BURST      3000        // Timeout of tcp burst write [mS]
#define h78TIMEOUT_UDP              10000       // Timeout of udp [mS]
#define h78TIMEOUT_CPWROFF          120000      // Timeout of power off [mS]
  // Misc..
#define h78IMEI_SIZE                15          // IMEI length[bytes] - '\0' is not included.
#define h78DATETIME_SIZE            19          // Date and time length[bytes] - '\0' is not included.
#define h78MAX_UDP_PAYLOAD_SIZE     1472        // Maximum payload size
#define h78MAX_TCP_DATA_SIZE_ONCE   32000       // Maximum data size that can be sent at one time via TCP
#define h78UNKNOWN_BODY_SIZE        (384*1024)  // Assume less than 384KB
#define h78MAX_RESULT_LENGTH        1088        // Actual maximum command result length, include '\n' (in byets)
#define h78MAX_RESPONSE_LENGTH      1023        // Maximum http/get or http/post response length (in byets) - compatible with v1
#define h78MAX_HOST_LENGTH          200         // Maximum length of host name with domain
#define h78MAX_PATH_SIZE            200         // Maxmimum path length of GET or POST
#define h78MAX_TCP_WRITE_SIZE       4096        // Maximum data size(in bytes) to write at once
#define h78MAX_TCP_READ_SIZE        4096        // Maximum data size(in bytes) to read at once
#define h78MAX_PORT_NUMBER          65535       // Maximum port number
#define h78BUFFER_SIZE              256         // Maximum data size(in bytes) in h78SEND* Macros
#define h78IP_V4_ADDRESS_LENGTH     15          // Size required to store IP(v4) address(included '\0')
//-- Error codes
  // Succeed(No error)
#define h78SUCCESS                  0           // When the call is successful
  // common errors
#define h78ERR_BAD_PARAM            900         // Bad parameters
#define h78ERR_TIMED_OUT            901         // Communication timed out
#define h78ERR_ERROR                902         // Unknown error
#define h78ERR_INTERNAL_ERROR       999         // Internal error or bug
#define h78ERR_ALREADY_INITIALIZED  100         //
#define h78ERR_NOT_YET_INITIALIZED  101         //
  // control function errors
#define h78ERR_CANOT_GET_RSSI       102         // getRSSI() -
#define h78ERR_CANOT_CONNECT        103         //
#define h78ERR_INVALID_DATETIME     104         // getDateTime() - invalid date and time (Probably not connected to LTE network)
#define h78ERR_CANOT_GET_IMEI       143         // getIMEI() -
#define h78ERR_CANOT_GET_DATETIME   122         // getDateTime() -
#define h78ERR_CANOT_SET_PROFILE    125         // setProfile() -
#define h78ERR_CANOT_ATTACH_LTE     199         // setProfile() -
  // http function errors
#define h78ERR_HTTP_SESSIONID       701         // doHttpGet()/doHttpPost() - セッションIDの取得に失敗した
#define h78ERR_HTTP_READY           702         // doHttpGet()/doHttpPost() - HTTPがレディとならない
#define h78ERR_HTTP_CONNECT         703         // doHttpGet()/doHttpPost() - サーバと接続できない
#define h78ERR_HTTP_HEADER          704         // doHttpGet()/doHttpPost() - ヘッダ送信時にエラーが発生した
#define h78ERR_HTTP_HEADER_RES      710         // doHttpGet()/doHttpPost() - レスポンスヘッダの取得・解析でエラーが発生した
#define h78ERR_HTTP_BODY_RES        711         // doHttpGet()/doHttpPost() - レスポンスボディの取得・解析でエラーが発生した
#define h78ERR_HTTP_GET             712         // doHttpGet()/doHttpPost() - GETの実行でエラーが発生した
#define h78ERR_HTTP_POST            720         // doHttpPost() - レスポンスヘッダの取得・解析でエラーが発生した
  // udp function errors
#define h78ERR_UDP_CONFIG           801         // beginUDP() -
#define h78ERR_UDP_TOO_BIG_DATA     802         // sendUDP() -
#define h78ERR_UDP_CONNECT          803         // sendUDP() -
#define h78ERR_UDP_RES              804         // sendUDP() -
#define h78ERR_UDP_NAME             805         // getNameUDP() -
  // tcp function errors
#define h78ERR_TCP_ALREADY_CONNECTED    601     //
#define h78ERR_TCP_NOT_YET_CONNECTED    602     //
#define h78ERR_TCP_CONFIG           603         //
#define h78ERR_TCP_DELETE           604         //
#define h78ERR_TCP_STATUS           620         // readTCP() -
#define h78ERR_TCP_CONNECT          637         // readTCP() -
#define h78ERR_TCP_READ             638         // readTCP() -
#define h78ERR_TCP_WRITE            639         //
#define h78ERR_TCP_STAT             641         // readTCP() -
#define h78ERR_TCP_ADDR             651         //
//-- TCP Status - return value from getStatusTCP()
#define h78TCPSTAT_NOT_DEFINED      0           //
#define h78TCPSTAT_CLOSED           1           //
#define h78TCPSTAT_DISCONNECTED     2           //
#define h78TCPSTAT_CONNECTING       3           //
#define h78TCPSTAT_CONNECTED        4           //
#define h78TCPSTAT_UNKNOWN          (-1)        //

//-- Macros
  // USBへデバッグプリントする
#ifdef DEBUG_USB
#   define h78USBDP(...)               { char _buf_[h78BUFFER_SIZE+1]; snprintf(_buf_, h78BUFFER_SIZE, __VA_ARGS__); SerialUSB.print(_buf_); SerialUSB.flush(); }
#   define h78USBDPLN(...)             { char _buf_[h78BUFFER_SIZE+1]; snprintf(_buf_, h78BUFFER_SIZE, __VA_ARGS__); SerialUSB.println(_buf_); SerialUSB.flush(); }
#   define h78USBDPC(c)                SerialUSB.print((c))
#   define h78USBDPFMT(v,f)            SerialUSB.print((v),(f))
#   define h78USBDPWRT(p,s)            SerialUSB.write((p),(s))
#   define h78USBFLUSH()               SerialUSB.flush()
#else
#   define h78USBDP(...)
#   define h78USBDPLN(...)
#   define h78USBDPC(c)
#   define h78USBDPFMT(v,f)
#   define h78USBDPWRT(p,s)
#   define h78USBFLUSH()
#endif // DEBUG
  // HL7800に文字列を送る
#define h78SEND(p,n)                   h78SERIAL.write((p),n)
#define h78SENDC(c)                    h78SERIAL.write(c)
#define h78SENDF(...)                  { char _buf_[h78BUFFER_SIZE+1]; snprintf(_buf_, h78BUFFER_SIZE, __VA_ARGS__); h78SERIAL.print(_buf_); }
#define h78SENDFLN(...)                { char _buf_[h78BUFFER_SIZE+1]; snprintf(_buf_, h78BUFFER_SIZE, __VA_ARGS__); h78SERIAL.println(_buf_); h78SERIAL.flush(); }

// Types and classes
  // Power off mode
typedef enum {
    POFF_NORMALLY,          // When turning off the power normally
    POFF_IMMEDIATELY        // When turning off the power in a hurry(Not recommended)
} POFF_MODE;

  // Date and time
typedef struct {
    uint8_t     day, month, year;
    uint8_t     hours, minutes, seconds;
} DATE_TIME;

  // Callback function
typedef void (*CALLBACK_FUNC)(void);

  // HL7800 class
class HL7800 {
  public:
    // Constructor with default parameters
    HL7800(int powerPin = _mgHL7800PowerPin, int powerOnPin = _mgHL7800PowerOnPin,
            int resetPin = _mgHL7800ResetPin, int wakeUpPin = _mgHL7800WakeUpPin):
            _powerPin(powerPin), _powerOnPin(powerOnPin), _resetPin(resetPin), _wakeUpPin(wakeUpPin) {
        _vgpioPin = _mgHL7800VGPIOPin;
        _initialized = false;
        _httpSessionId = _tcpSessionId = _udpSessionId = 0;
    }

    // Begin/end
    int begin(void);
    int end(void);

    // System Control
    int getIMEI(char imei[]);
    int getDateTime(char datetime[]);
    int getDateTime(DATE_TIME *datetime);
    int getRSSI(int *rssi);
    int getService(int *state);
    int setProfile(char *apn, char *user, char *password);
    int powerOff(POFF_MODE mode = POFF_NORMALLY);
    int powerOn(void);
    boolean isPowerOn(void);
    int reset(void);        // Not implemented(R1)
    int doAT(char *at);     // Not implemented(R1)

    // UDP communication
    int beginUDP(void);
    int endUDP(void);
    int sendUDP(char *server, int port, void *msg, int size);
    int getNameUDP(char *ip);
    // int getStatusUDP(int *status, int *tcpNotif, int *remainedBytes, int *recievedBytes);

    // TCP communication
    int connectTCP(const char *host, int port);
    int disconnectTCP(void);
    int readTCP(void *msg, int size);
    int writeTCP(const void *msg, int size);
    int writeTCP(const char *buf) {
        return (writeTCP((const void *)buf, strlen(buf)));
    };
    int writeBurstTCP(int size);
    int getNameTCP(char *ip);
    int getStatusTCP(int *status, int *tcpNotif, int *remainedBytes, int *recievedBytes);
    int configureTCP(uint32_t timeout_connect, uint32_t timeout_write);  // Not implemented (R1.0)
    int getTimeoutToConnectTCP(void) {
        return (_timeoutTcpConnect);
    }
    int getTimeoutToWriteTCP(void) {
        return (_timeoutTcpWrite);
    }

    // HTTP/HTTPS functions
    int doHttpGet(char *url, char *header, char *response, int *responseSize);
    int doHttpPost(char *url, char *header, void *body, int bodySize, char *response, int *responseSize);
    // int requestHttpPost(char *url, char *header, char *body, CALLBACK_FUNC handleResponse);

    // Misc.
    int getResponse(uint32_t timeout, char *response, int *size);

  private:
    // Methods
    void discardResponse(uint32_t timeout);
    int getSessionId(uint32_t timeout, const char *ind, int *udpSessionId);
    int waitUntilCONNECT(uint32_t timeout);
    int getLine(uint32_t limit, char *line, int size);
    int parseCGATT(int *state);
    int parseCCLK(char *resp, char *datetime);
    int waitUntilReady(uint32_t timeout, const char *prefix, int sessionId);
    int parseKTCPSTAT(int *status, int *tcpNotif, int *remainedBytes, int *recievedBytes);
    int	parseKCGPADDR(char *ipAddress);
    int getData(uint32_t timeout, char *resp, int *size);
    int splitUrl(char *url, char *host, int *port, char *path, int *useSSL);
    int parseHeader(int *httpStatus, int *contentLength);
    int getBody(char *response, int *len, int contentLength, uint32_t timeout);
    void hangUp(void);
    void closeHttpSession(void);
    boolean isEOD(char *latests, int size, int last);

    // Variables
    boolean _initialized;
      // Session IDs (0 if there is no session)
    int _udpSessionId;
    int _tcpSessionId;
    int _httpSessionId;
      // time out values
    int _timeoutTcpConnect;
    int _timeoutTcpWrite;
     // HL7800 control pins
    int _powerPin, _powerOnPin, _resetPin, _wakeUpPin, _vgpioPin;
    const int _RTS = 24;        // [out] RTS (Active:LOW)
    const int _CTS = 24;        // [in] CTS
};

#endif // _hl7800_h_
