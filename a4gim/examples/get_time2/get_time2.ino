// 4GIM(V1) sample sketch for Uno/Pro.. -- getTime2
//   for example, check result by http://www.epochconverter.com/

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
    uint32_t seconds;
    if (a4gs.getTime2(seconds) == 0) {
      Serial.print(seconds);
      Serial.println(" Sec.");
    }
    else
      Serial.println("Can't get seconds.");  
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
