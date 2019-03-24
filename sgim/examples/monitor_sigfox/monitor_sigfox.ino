/*
 * Sample skect for SGIM - Sigfox Monitor
 *
 * [説明]
 *  SGIMに搭載されているSigfox通信モジュールSFM10R3のIDとPACをシリアルモニタに表示する。
 *  その後、ATコマンドの入力待ちとなる。ATコマンドを入力すると、
 *  そのレスポンスがシリアルモニタに表示される。 
 *
 * [補足]
 *  ATコマンドの詳細は、WISOL製SFM1R3ATの仕様書を参考のこと。
 *
 */

#include <sgim.h>

const unsigned long baudrate = BAUDRATE_SIGFOX;

void setup() {
  while (! SerialUSB)
    ;
  SerialUSB.begin(baudrate);

  // Set up iotab
  if (! sgim.begin()) {
    SerialUSB.println("sgim.begin(): error.");
    while (1)
      ;       // Stop here
  }

  String id;
  if (sgim.getID(id)) {
      SerialUSB.print("ID: ");
      SerialUSB.println(id);
  }

  String pac;
  if (sgim.getPAC(pac)) {
      SerialUSB.print("PAC: ");
      SerialUSB.println(pac);
  }

  sgSerial.begin(baudrate);

  SerialUSB.println("Ready.");
}

void loop() {
  if (sgSerial.available() > 0) {
    char c = sgSerial.read();
    SerialUSB.print(c);
  }

  if (SerialUSB.available() > 0) {
    char c = SerialUSB.read();
    sgSerial.print(c);
    SerialUSB.print(c);    // Echo back
  }  
  
}
