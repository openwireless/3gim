// 4GIM(V1) sample sketch for Mega/Leonardo.. -- getServices

#include "a4gim2.h"

const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");

  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
    Serial.println("Succeeded.");
    int status;
    if (a4gs.getServices(status) == 0) {
      switch (status) {
        case a4gsSRV_NO :
          Serial.println("No Service.");
          break;
        case a4gsSRV_PS :
          Serial.println("Packet Service Only.");
          break;
        case a4gsSRV_CS :		// Never returned at R4.0
          Serial.println("Voice Service Only.");    // also SMS
          break;
        case a4gsSRV_BOTH :	// Never returned at R4.0
          Serial.println("Packet And Voice Services.");
          break;
        default :
          break;
      }
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
