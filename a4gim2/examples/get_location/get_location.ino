// 3GIM(V2) sample sketch for Mega/Leonardo.. -- getLocation

#include "a3gim2.h"

#define baudrate 	9600UL
const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(baudrate);
  delay(3000);    // Wait for start serial monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded. It maybe takes several minutes.");
    char lat[15], lng[15];
    if (a3gs.getLocation(a3gsMPBASED, lat, lng) == 0) {
      Serial.print("OK: ");
      Serial.print(lat);
      Serial.print(", ");
      Serial.println(lng);
    }
    else
      Serial.println("Sorry, I don't know this location.");
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
