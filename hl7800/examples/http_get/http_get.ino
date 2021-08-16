/*
 *  hl7800 usage and http/get - sample sketch
 */

#include <mgim.h>
#include <hl7800.h>

// アクセス先のURL
#define URL   "http://tabrain.jp/new/company.html"

HL7800  hl7800;

void setup() {
  // 最初に、mgimの初期化
  mgim.begin();

  while (! mgSERIAL_MONITOR)
    ;
  mgSERIAL_MONITOR.begin(9600);
  mgSERIAL_MONITOR.println("Start..");

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

  // HTTP/GETの実行、レスポンスの表示
  char resp[1000];
  int respSize = sizeof(resp) - 1;
  if ((stat = hl7800.doHttpGet(URL, NULL, resp, &respSize)) != 0) {
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
