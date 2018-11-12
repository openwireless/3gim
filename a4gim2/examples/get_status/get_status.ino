// 3GIM(V2) sample sketch for Mega/Leonardo.. -- getStatus

#include <a3gim2.h>

#define baudrate 	9600UL
const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)

void setup()
{
  Serial.begin(baudrate);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");

  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
    int  status;
    status = a3gs.getStatus();
    Serial.print("Status is ");
    switch (status) {
      case A3GS::ERROR :
        Serial.println("ERROR");
        break;
      case A3GS::IDLE :
        Serial.println("IDLE");
        break;
      case A3GS::READY :
        Serial.println("READY");
        break;
      case A3GS::TCPCONNECTEDCLIENT :
        Serial.println("TCPCONNECTEDCLIENT");
        break;
      default :
        Serial.println("Unknown");
        break;
    }
  }   

  Serial.println("Shutdown..");
  a3gs.end();
  a3gs.shutdown();
}

void loop()
{
}

// END
