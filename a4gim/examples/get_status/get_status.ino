// 4GIM(V1) sample sketch for Uno/Pro.. -- getStatus

#include <a4gim.h>

const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");

  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
    Serial.println("Succeeded.");
    int  status;
    status = a4gs.getStatus();
    Serial.print("Status is ");
    switch (status) {
      case A4GS::ERROR :
        Serial.println("ERROR");
        break;
      case A4GS::IDLE :
        Serial.println("IDLE");
        break;
      case A4GS::READY :
        Serial.println("READY");
        break;
      case A4GS::TCPCONNECTEDCLIENT :
        Serial.println("TCPCONNECTEDCLIENT");
        break;
      default :
        Serial.println("Unknown");
        break;
    }
  }   

  Serial.println("Shutdown..");
  a4gs.end();
  a4gs.shutdown();
}

void loop()
{
}

// END
