// 3GIM(V2) sample sketch for Mega/Leonardo.. -- setBaudrate
//
//   You need to use "setBaudrate()" function CAREFULLY. 
//   Because, 3G Shield becomes impossible to use when setting up 
//   the value which is not suitable. 
//   This function is used when gathering the communication speed of 
//   Arruino and 3GIM Shield using HardwareSerial instead of SoftwareSerial.
//   In order for you to use HardwareSerial, you need to correct "a3gs.h" 
//   and "a3gim.cpp" appropriately.
//   When using an "a3gim" library after performing this sketch, 
//   begin() is called in the following arguments: 
//       begin(0, NEW_BAUDRATE);

#include "a3gim2.h"

#define NEW_BAUDRATE  		38400
#define CURRENT_BAUDRATE  9600
const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(CURRENT_BAUDRATE);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, CURRENT_BAUDRATE) == 0) {
//@  if (a3gs.start() == 0 && a3gs.begin(0, NEW_BAUDRATE) == 0) {
    Serial.println("Succeeded.");
    if (a3gs.setBaudrate(NEW_BAUDRATE) == 0) {
      Serial.print("Baudrate was changed as ");
      Serial.print(NEW_BAUDRATE);
      Serial.println(" bps.");
    }
  }
  else
    Serial.println("Failed, baudrate was not changed.");

  Serial.println("Shutdown..");
  a3gs.end();
  a3gs.shutdown();
}

void loop()
{
}

// END
