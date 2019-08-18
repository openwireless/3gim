// 4GIM(V1) sample sketchfor Uno/Pro.. -- check current baudrate

#include "a4gim.h"

const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)
long baudrates[7] = { 9600, 19200, 38400, 57600, 115200 };
	//-- 2400bps and 4800bps are not supported in 4GIM(V1)

void setup()
{
  Serial.begin(9600);
  Serial.println("Ready.");

  Serial.println("Initializing.. ");
  for (int i = 0; i < 7; i++) {
    if (a4gs.start(powerPin) == 0) {
      Serial.print("Try baudrate: ");
      Serial.println(baudrates[i]);
      if (a4gs.begin(0, baudrates[i]) == 0) {
        Serial.println("Recognize succeeded.");
        Serial.print("Current baudrate is ");
        Serial.print(baudrates[i]);
        Serial.println(" bps.");
        return;
      }
      Serial.println("Failed.");
      a4gs.end();
      a4gs.shutdown();
    }
  }
  Serial.println("Can't recognize baudrate.");
}

void loop()
{
}

// END
