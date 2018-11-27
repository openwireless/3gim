// 4GIM(V1) sample sketch for Mega/Leonardo/Zero.. -- setLED

#include <a4gim2.h>

#define  INTERVAL  1000     // Blink interval
const int powerPin = 7;     // 4gim power pinIf not using power control, 0 is set.

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  boolean done = false;
  while (! done) {
    Serial.print("Initializing.. ");
    if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
      Serial.println("Succeeded.");
	  done = true;
    }
    else {
      Serial.println("Failed.");
      Serial.println("Shutdown..");
      a4gs.end();
      a4gs.shutdown();
      delay(30000);
    }    
  }

  Serial.println("Blinking..");
}

void loop()
{
  a4gs.setLED(true);
  delay(INTERVAL);
  a4gs.setLED(false);
  delay(INTERVAL);
}

// END