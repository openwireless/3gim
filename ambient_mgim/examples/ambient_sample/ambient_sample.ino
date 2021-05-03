/*
 *  Ambient sample skect with mgim, hl7800 and stts751(on board temperature sensor)
 */

#include <mgim.h>
#include <hl7800.h>
#include <Wire.h>
#include <stts751.h>
#include "ambient_mgim.h"

// Set your REAL channel-id and write-key
#define CHANNEL_ID  ***
#define WRITE_KEY   "@@@"

HL7800  hl7800;
STTS751 stts751;
Ambient_mgim am(hl7800);

void setup() {
  while (! mgSERIAL_MONITOR)
    ;
  mgSERIAL_MONITOR.begin(115200);
  mgSERIAL_MONITOR.println("Start.");

  Wire.begin();
  stts751.begin();

  am.begin(CHANNEL_ID, WRITE_KEY);

  hl7800.powerOn();
  delay(1000);

  int stat = hl7800.begin();
  if (stat != 0) {
    mgSERIAL_MONITOR.println("hl7800(): error");
    while (1) ;
  }

  mgSERIAL_MONITOR.println("setup(): done");
}

void loop() {
  float t = stts751.getTemperature() / 10.0;
  mgSERIAL_MONITOR.print("uploading: ");
  mgSERIAL_MONITOR.println(t, 1);
  am.set(1, t);
  if (am.send())
    mgSERIAL_MONITOR.println("send(): ok");
  else
    mgSERIAL_MONITOR.println("send(): error");

  delay(10000);
}
