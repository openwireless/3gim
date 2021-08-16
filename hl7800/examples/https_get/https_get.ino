/*
 *  hl7800 usage and https/get - sample sketch
 */

#include <mgim.h>
#include <hl7800.h>

// アクセス先のURL
#define URL   "https://arduino.cc/"
//#define URL   "https://store.arduino.cc/usa/arduino-uno-rev3"

char *rootCA_arduino_cc =
"-----BEGIN CERTIFICATE-----\r\n" \
"MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\r\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\r\n" \
"MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\r\n" \
"AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\r\n" \
"jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\r\n" \
"Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\r\n" \
"U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\r\n" \
"gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\r\n" \
"/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\r\n" \
"oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\r\n" \
"BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\r\n" \
"ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\r\n" \
"p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\r\n" \
"AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\r\n" \
"Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\r\n" \
"LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\r\n" \
"r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\r\n" \
"AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\r\n" \
"ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\r\n" \
"S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\r\n" \
"qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\r\n" \
"O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\r\n" \
"UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\r\n" \
"-----END CERTIFICATE-----\r\n";
char *header = "HOST: arduino.cc";

HL7800  hl7800;

void setup() {
  // 最初に、mgimの初期化
  mgim.begin();

  while (! mgSERIAL_MONITOR)
    ;
  mgSERIAL_MONITOR.begin(9600);
  mgSERIAL_MONITOR.println("Start..");

  mgSERIAL_MONITOR.print(">>");
  mgSERIAL_MONITOR.print(rootCA_arduino_cc);
  mgSERIAL_MONITOR.println("<<");

  // hl7800の電源Onと初期化
  hl7800.powerOn();
  delay(1000);
  int stat = hl7800.begin();
  if (stat != 0) {
    mgSERIAL_MONITOR.println("hl7800(): error");
    while (1) ;
  }

  //    IMEIの取得・表示
  char imei[h78IMEI_SIZE];
  if (hl7800.getIMEI(imei) != 0) {
    mgSERIAL_MONITOR.println("getIMEI() error");
  }
  else {
    mgSERIAL_MONITOR.print("imei=");
    mgSERIAL_MONITOR.println(imei);
  }

  // 現在日時の取得・表示
  char datetime[100];
  int retryCount = 3;
  while (retryCount-- > 0 && hl7800.getDateTime(datetime) != 0)
    delay(3000);
  if (retryCount == 0) {
    mgSERIAL_MONITOR.println("getDateTime() error");
  }
  else {
    mgSERIAL_MONITOR.print("date&time=");
    mgSERIAL_MONITOR.println(datetime);
  }

  // APNの設定
  hl7800.setProfile("soracom.io", "sora", "sora");

  // RSSIの取得・表示
  int rssi = 0;
  retryCount = 3;
  while (retryCount-- > 0 && hl7800.getRSSI(&rssi) != 0)
    delay(3000);
  if (retryCount == 0) {
    mgSERIAL_MONITOR.println("getRSSI() error");
  }
  else {
    mgSERIAL_MONITOR.print("rssi=");
    mgSERIAL_MONITOR.print(rssi);
    mgSERIAL_MONITOR.println(" dBm");
  }

  mgim.setLed(1);

  // HTTPS/GETの実行、レスポンスの表示
  if ((stat = hl7800.setRootCA(rootCA_arduino_cc)) != h78SUCCESS) {
    mgSERIAL_MONITOR.print("setRootCA() error: ");
    mgSERIAL_MONITOR.println(stat);
  }
  else {
    char resp[1000];
    int respSize = sizeof(resp) - 1;
    if ((stat = hl7800.doHttpGet(URL, header, resp, &respSize)) != h78SUCCESS) {
      mgSERIAL_MONITOR.print("doHttpGet() error: ");
      mgSERIAL_MONITOR.println(stat);
    }
    else {
      mgSERIAL_MONITOR.print("http status=");
      mgSERIAL_MONITOR.println(hl7800.getLastHttpStatusCode());
      mgSERIAL_MONITOR.print("body=");
      mgSERIAL_MONITOR.print(respSize, DEC);
      mgSERIAL_MONITOR.print(",\"");
      mgSERIAL_MONITOR.write(resp, respSize);
      mgSERIAL_MONITOR.println("\"");
    }
  }

  // hl7800の電源Offと終了
  mgSERIAL_MONITOR.print("power off hl7800: ");
  hl7800.powerOff();
  hl7800.end();
  mgSERIAL_MONITOR.println("done");
    // 確実に電源がOffとなるまで待つ
  while (hl7800.isPowerOn())
    delay(100);

  mgSERIAL_MONITOR.println("Done.");
  mgim.setLed(0);
}

void loop() {
  // do nothing
}
