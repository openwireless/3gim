// 4GIM(V1) sample sketch for Uno/Pro.. -- tweet
//  [[Note]] REPLACE "token" and "message" WITH YOUR GOT TOKEN AND MESSAGE BEFORE UPLOAD THIS SKETCH.
//           THIS TEST SKETCH USE http://arduino-tweet.appspot.com/ 's SERVICE. PLEASE CHECK DETAIL IN THIS SITE.

#include "a4gim.h"

const int powerPin = 7;     // 4gim power pinIf not using power control, 0 is set.
const char *token = "YOUR_TOKEN_HERE";
const char *message = "TWEET_MESSAGE_HERE";
  //-- Note: can't tweet same message continuously.

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
    Serial.println("Succeeded.");
    Serial.print("tweet() requesting.. ");
    if (a4gs.tweet(token, message) == 0)
      Serial.println("OK!");
    else
      Serial.println("Can't tweet.");
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
