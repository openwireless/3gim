// 4GIM(V1) sample sketch for Mega/Leonardo.. -- getTime

#include "a4gim2.h"

#define baudrate 	9600UL
const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(baudrate);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
    char date[a4gsDATE_SIZE], time[a4gsTIME_SIZE];
    if (a4gs.getTime(date, time) == 0) {
      Serial.print(date);
      Serial.print(" ");
      Serial.println(time);
    }
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