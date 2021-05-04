/*
 * UDP send sample sketch
 *
 *  対向するサーバは、UDPポートPORT_NOで待ち受けして、文字列データを受信するものとする
 */

#include <mgim.h>
#include <hl7800.h>

#define URL         "***.***"   // UDPパケットの送付先のホスト(FQDN or IPアドレス)
#define PORT_NO     8080        // UDPパケットの送付先のポート番号

HL7800  hl7800;
char data[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";

void setup() {
  // 最初に、mgimの初期化
  mgim.begin();

  while (! mgSERIAL_MONITOR)
    ;
  mgSERIAL_MONITOR.begin(9600);
  mgSERIAL_MONITOR.println("UDP TEST Start..");

  hl7800.powerOn();
  delay(1000);

  int stat = hl7800.begin();
  if (stat != 0) {
    mgSERIAL_MONITOR.println("hl7800(): error");
    while (1) ;
  }
  mgSERIAL_MONITOR.println("hl7800(): OK");

  if ((stat = hl7800.setProfile("soracom.io", "sora", "sora")) == 0) {
    mgSERIAL_MONITOR.println("setProfile() OK");
  }
  else {
    mgSERIAL_MONITOR.print("setProfile() error: ");
    mgSERIAL_MONITOR.println(stat);
    while (1) ;
  }

  if ((stat = hl7800.beginUDP()) == 0) {
    mgSERIAL_MONITOR.println("beginUDP() OK");
  }
  else {
    mgSERIAL_MONITOR.print("beginUDP() error: ");
    mgSERIAL_MONITOR.println(stat);
    while (true)
      ;
  }
}

void loop() {
  int stat;
  mgSERIAL_MONITOR.println(">>");
  if ((stat = hl7800.sendUDP(URL, PORT_NO, (void *)data, (int)sizeof(data))) == 0) {
    mgSERIAL_MONITOR.println("sendUDP() OK");
    delay(200);
  }
  else {
    mgSERIAL_MONITOR.print("sendUDP() error: ");
    mgSERIAL_MONITOR.println(stat);
    delay(200);
    hl7800.endUDP();
    if ((stat = hl7800.beginUDP()) != 0)
      delay(1000);
  }
  mgSERIAL_MONITOR.println("<<");

  delay(3000);
}
