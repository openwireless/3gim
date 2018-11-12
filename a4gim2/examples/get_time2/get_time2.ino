// 3GIM(V2) sample sketch for Mega/Leonardo.. -- getTime2
//   check result by http://www.epochconverter.com/

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
    uint32_t seconds;
    if (a3gs.getTime2(seconds) == 0) {
      Serial.print(seconds);
      Serial.println(" Sec.");
    }
    else
      Serial.println("Can't get seconds.");  
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
