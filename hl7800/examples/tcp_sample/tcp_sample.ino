/*
 * TCP function sample sketch
 *
 *  対向するサーバは、echo server（受信した文字列データをそのままレスポンスとして返すサーバ）とする
 */

#include <mgim.h>
#include <hl7800.h>

#define HOST    "***.***"   // エコーサーバのFQDNまたはIPアドレス
#define PORT    8888        // TCPパケットの送付先のポート番号(サーバの待ち受けポート番号)

const char data[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";   // サーバへ送るデータ
HL7800  hl7800;

void setup() {
  // 最初に、mgimの初期化
  mgim.begin();

  while (! mgSERIAL_MONITOR)
    ;
  mgSERIAL_MONITOR.begin(9600);
  mgSERIAL_MONITOR.println("TCP Sample Start..");

  // HL7800の電源を入れて、ライブラリhl7800を初期化する
  hl7800.powerOn();
  delay(1000);
  int stat = hl7800.begin();
  if (stat != 0) {
    mgSERIAL_MONITOR.println("hl7800(): error");
    while (1) ;
  }
  mgSERIAL_MONITOR.println("hl7800(): OK");

  // APNをセットする(使用するSIMに合わせて変更すること)
  if ((stat = hl7800.setProfile("soracom.io", "sora", "sora")) == 0) {
    mgSERIAL_MONITOR.println("setProfile() OK");
  }
  else {
    mgSERIAL_MONITOR.print("setProfile() error: ");
    mgSERIAL_MONITOR.println(stat);
    while (1) ;
  }

  // サーバにTCPで接続する
  int retryCount = 3;
  while (retryCount-- > 0 && (stat = hl7800.connectTCP(HOST, PORT)) != 0)
    delay(3000);

  if (retryCount > 0) {
    mgSERIAL_MONITOR.println("connectTCP() OK");
  }
  else {
    mgSERIAL_MONITOR.print("connectTCP() error: ");
    mgSERIAL_MONITOR.println(stat);
    while (true)
      ;             // Stop here - 接続に失敗したら実行を止める
  }
}

void loop() {
  int stat;
  mgSERIAL_MONITOR.println(">>");

  if ((stat = hl7800.writeTCP((void *)data, strlen(data))) > 0) {
    mgSERIAL_MONITOR.print("writeTCP() OK: ");
    mgSERIAL_MONITOR.println(stat);
    delay(500);

    // レスポンスを読み取る
    int respBytes = stat, readBytes = 0;
    char resp[100];
    while (readBytes < respBytes) {
      char buf[200];
      if ((stat = hl7800.readTCP((void *)buf, (respBytes - readBytes))) > 0) {
        mgSERIAL_MONITOR.print("readTCP() OK: ");
        mgSERIAL_MONITOR.println(stat);
        memcpy(resp+readBytes, buf, stat);
        readBytes += stat;
      }
      else if (stat == 0) {
        mgSERIAL_MONITOR.println("readTCP() Done.");
        break;      // exit from while-loop as succeed
      }
      else {  // stat < 0
        mgSERIAL_MONITOR.print("readTCP() NG: ");
        mgSERIAL_MONITOR.println(stat);
        break;      // exit from while-loop as failed
      }
    }
    resp[readBytes] = '\0';
    mgSERIAL_MONITOR.print("[[");
    mgSERIAL_MONITOR.write(resp, readBytes);
    mgSERIAL_MONITOR.print("]]");
  }
  else {
    // writeTCP()がエラーの時
    mgSERIAL_MONITOR.print("writeTCP() error: ");
    mgSERIAL_MONITOR.println(stat);
    delay(200);

    // 一度、TCPコネクションを切断して、再接続を試みる
    hl7800.disconnectTCP();

    int retryCount = 3;
    while (retryCount-- > 0 && (stat = hl7800.connectTCP(HOST, PORT)) != 0)
      delay(3000);
    if (retryCount > 0) {
      mgSERIAL_MONITOR.println("connectTCP() OK");
    }
    else {
      mgSERIAL_MONITOR.print("connectTCP() error: ");
      mgSERIAL_MONITOR.println(stat);
      while (true)
        ;               // Stop here - 接続に失敗したら実行を止める
    }
  }

  mgSERIAL_MONITOR.println("<<");

  delay(3000);
}
