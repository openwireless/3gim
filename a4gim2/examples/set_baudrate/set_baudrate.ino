// 4GIM(V1) sample sketch for Mega/Leonardo.. -- setBaudrate
//
//   You need to use "setBaudrate()" function CAREFULLY. 
//   Because, 3G Shield/3GIM/4GIM becomes impossible to use when setting up 
//   the value which is not suitable. 
//   This function is used when gathering the communication speed of 
//   Arruino and 3GIM/4GIM Shield using HardwareSerial instead of SoftwareSerial.
//   In order for you to use HardwareSerial, you need to correct "a4gs.h" 
//   and "a4gim.cpp" appropriately.
//   When using an "a4gim2" library after performing this sketch, 
//   begin() is called in the following arguments: 
//       begin(0, NEW_BAUDRATE);

#include "a4gim2.h"

#define NEW_BAUDRATE  		9600
#define CURRENT_BAUDRATE    115200
const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin(0, CURRENT_BAUDRATE) == 0) {
    Serial.println("Succeeded.");
    if (a4gs.setBaudrate(NEW_BAUDRATE) == 0) {
      Serial.print("Baudrate was changed as ");
      Serial.print(NEW_BAUDRATE);
      Serial.println(" bps.");
    }
  }
  else
    Serial.println("Failed, baudrate was not changed.");

  Serial.println("Shutdown..");
  a4gs.end();
  a4gs.shutdown();
}

void loop()
{
}

// END
