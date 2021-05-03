/*
 *  hl7800 usage and http/post - sample sketch
 */

#include <mgim.h>
#include <hl7800.h>

// アクセス先のURL
#define URL   "http://***.***.***/"

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

  // APNの設定
  hl7800.setProfile("soracom.io", "sora", "sora");

  mgim.setLed(1);

  // HTTP/POSTの実行、レスポンスの表示
  char resp[300];
  int respSize = sizeof(resp) - 1;
  char body[100];
  sprintf(body, "My IMEI is %s", imei);
  int bodySize = strlen(body);
  char header[100];
  sprintf(header, "content-length:%d", bodySize);
  if ((stat = hl7800.doHttpPost(URL, header, (void *)body, bodySize, resp, &respSize)) != 0) {
    mgSERIAL_MONITOR.print("doHttpPost() error: ");
    mgSERIAL_MONITOR.println(stat);
  }
  else {
    mgSERIAL_MONITOR.print("body=\"");
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
