// 3GIM(V2) sample sketch for Mega/Leonardo.. -- getServices

#include "a3gim2.h"

#define baudrate 	9600UL
const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(baudrate);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");

  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
    int status;
    if (a3gs.getServices(status) == 0) {
      switch (status) {
        case a3gsSRV_NO :
          Serial.println("No Service.");
          break;
        case a3gsSRV_PS :
          Serial.println("Packet Service Only.");
          break;
        case a3gsSRV_CS :		// Never returned at R4.0
          Serial.println("Voice Service Only.");    // also SMS
          break;
        case a3gsSRV_BOTH :	// Never returned at R4.0
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
  a3gs.end();
  a3gs.shutdown();
}

void loop()
{
}

// END
