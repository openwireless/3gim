// 3GIM(V2) sample sketch for Mega/Leonardo.. -- tweet
//  [[Note]] REPLACE "token" and "message" WITH YOUR GOT TOKEN AND MESSAGE BEFORE UPLOAD THIS SKETCH.
//           THIS TEST SKETCH USE http://arduino-tweet.appspot.com/ 's SERVICE. PLEASE CHECK DETAIL IN THIS SITE.

#include "a3gim2.h"

#define baudrate 	9600UL
const int powerPin = 7;     // 3gim power pinIf not using power control, 0 is set.
const char *token = "YOUR_TOKEN_HERE";
const char *message = "TWEET_MESSAGE_HERE";
  //-- Note: can't tweet same message continuously.

void setup()
{
  Serial.begin(baudrate);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
    Serial.print("tweet() requesting.. ");
    if (a3gs.tweet(token, message) == 0)
      Serial.println("OK!");
    else
      Serial.println("Can't tweet.");
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
