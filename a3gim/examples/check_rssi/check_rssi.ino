// 3GIM(V2) sample skech -- getRSSI

#include "a3gim.h"

#define baudrate 	9600UL
const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(baudrate);
  delay(100);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
    int rssi;
    if (a3gs.getRSSI(rssi) == 0) {
      Serial.print("RSSI = ");
      Serial.print(rssi);
      Serial.println(" dBm");
    }
  }
  else
    Serial.println("Failed.");

  Serial.println("Shutdown..");
  a3gs.shutdown();
  a3gs.end();
}

void loop()
{
}

// END
