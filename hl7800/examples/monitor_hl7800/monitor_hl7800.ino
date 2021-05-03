/*
 *  Execute AT command on hl7800 - sample sketch
 */

#include <mgim.h>
#include <hl7800.h>

const uint32_t  baudrate = h78BAUDRATE;

HL7800  hl7800;

void setup() {
  // 最初に、mgimの初期化
  mgim.begin();

  while (! mgSERIAL_MONITOR)
    ;
  mgSERIAL_MONITOR.begin(baudrate);
  mgSERIAL_MONITOR.println("Start..");

  // hl7800の電源Onと初期化
  hl7800.powerOn();
  delay(1000);
  int stat = hl7800.begin();
  if (stat != 0) {
    mgSERIAL_MONITOR.println("hl7800(): error");
    while (1) ;
  }

  // hl7800がアイドル状態でもスリープしないようにしておく
  h78SERIAL.println("AT+KSLEEP=2");
  h78SERIAL.flush();
}

void loop() {
  if (h78SERIAL.available() > 0) {
    char c = h78SERIAL.read();
    mgSERIAL_MONITOR.print(c);
  }

  if (mgSERIAL_MONITOR.available() > 0) {
    char c = mgSERIAL_MONITOR.read();
    h78SERIAL.print(c);
  }
}
