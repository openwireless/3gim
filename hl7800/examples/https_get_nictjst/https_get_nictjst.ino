/*
 *  hl7800 usage and https/get - sample sketch
 */

#include <mgim.h>
#include <hl7800.h>

// アクセス先のURL(NICTが日本標準時を表示するページ)
#define URL   "https://www.nict.go.jp/JST/JST5.html"

char *rootCA_nict =
"-----BEGIN CERTIFICATE-----\r\n" \
"MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G\r\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp\r\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4\r\n" \
"MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG\r\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\r\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8\r\n" \
"RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT\r\n" \
"gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm\r\n" \
"KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd\r\n" \
"QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ\r\n" \
"XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw\r\n" \
"DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o\r\n" \
"LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU\r\n" \
"RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp\r\n" \
"jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK\r\n" \
"6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX\r\n" \
"mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs\r\n" \
"Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH\r\n" \
"WD9f\r\n" \
"-----END CERTIFICATE-----\r\n";

char *header = "User-Agent: Arduino";

HL7800  hl7800;

void setup() {
  // 最初に、mgimの初期化
  mgim.begin();

  while (! mgSERIAL_MONITOR)
    ;
  mgSERIAL_MONITOR.begin(9600);
  mgSERIAL_MONITOR.println("Start..");

  // nict.go.jpのルート証明書(Root CA)を表示
  mgSERIAL_MONITOR.print(">>");
  mgSERIAL_MONITOR.print(rootCA_nict);
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
  if ((stat = hl7800.setRootCA(rootCA_nict)) != h78SUCCESS) {
    mgSERIAL_MONITOR.print("setRootCA() error: ");
    mgSERIAL_MONITOR.println(stat);
  }
  else {
    char resp[2000];
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
