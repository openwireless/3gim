// 4GIM(V1) sample skech for Uno/Pro.. -- getRSSI

#include "a4gim.h"

const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
    Serial.println("Succeeded.");
    int rssi;
    if (a4gs.getRSSI(rssi) == 0) {
      Serial.print("RSSI = ");
      Serial.print(rssi);
      Serial.println(" dBm");
    }
  }
  else
    Serial.println("Failed.");

  Serial.println("Shutdown..");
  a4gs.end();
  a4gs.shutdown();
}

void loop()
{
}

// END
