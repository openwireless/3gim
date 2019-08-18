// 4GIM(V1) sample sketch for Uno/Pro.. -- setDefaultProfile and getDefaultProfile

#include "a4gim.h"

const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)
  // New profile -- for example, SORACOM SIM
const char *newApn = "soracom.io";
const char *newUser = "sora";
const char *newPassword = "sora";

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
    Serial.println("Succeeded.");
	// Get current default profile
    char apn[20], user[20], password[20];
    if (a4gs.getDefaultProfile(apn, user, password) == 0) {
      Serial.print("Default Profile is ");
      Serial.print(apn);
      Serial.print(",");
      Serial.print(user);
      Serial.print(",");
      Serial.println(password);
    }

	// Set new default profile
    if (a4gs.setDefaultProfile(newApn, newUser, newPassword) == 0) {
      Serial.println("Succeed.");
    }
    else
      Serial.println("Failed.");

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
