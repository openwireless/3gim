// 3GIM(V2) sample sketch for Mega/Leonardo/Zero.. -- getLocation2

#include "a3gim2.h"

#define baudrate 	9600UL
#define	TIMEOUT	    9000    // Time to wait until the positioning is over [10mS]

const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)
const boolean useAGPS = true;            // Use AGPS function
const boolean useActiveAntenna = false;  // Not use active GPS antenna

void setup()
{
  Serial.begin(baudrate);
  delay(3000);    // Wait for start serial monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
	
    // Set timeout and make AGPS function available
    a3gs.setLocationParams(TIMEOUT, useAGPS, useActiveAntenna);
	
    // Get location with specified timeout
    char lat[15], lng[15], utc[7], height[8];
    int quality, number;
    if (a3gs.getLocation2(lat, lng, height, utc, &quality, &number) == 0) {
      Serial.print("OK: ");
      Serial.print(lat);
      Serial.print(",");
      Serial.print(lng);
      Serial.print(",");
      Serial.print(height);
      Serial.print(",");
      Serial.print(utc);
      Serial.print(",");
      Serial.print(quality);
      Serial.print(",");
      Serial.println(number);
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
