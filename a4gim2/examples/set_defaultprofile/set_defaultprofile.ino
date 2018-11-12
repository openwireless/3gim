// 3GIM(V2) sample sketch for Mega/Leonardo.. -- setDefaultProfile and getDefaultProfile

#include "a3gim2.h"

#define baudrate 	9600UL
const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)
  // New profile
const char *newApn = "x.y";
const char *newUser = "u";
const char *newPassword = "p";

void setup()
{
  Serial.begin(baudrate);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
	// Get current default profile
    char apn[20], user[20], password[20];
    if (a3gs.getDefaultProfile(apn, user, password) == 0) {
      Serial.print("Default Profile Number is ");
      Serial.print(apn);
      Serial.print(",");
      Serial.print(user);
      Serial.print(",");
      Serial.println(password);
    }

	// Set new default profile
    if (a3gs.setDefaultProfile(newApn, newUser, newPassword) == 0) {
      Serial.println("Succeed.");
    }
    else
      Serial.println("Failed.");

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
