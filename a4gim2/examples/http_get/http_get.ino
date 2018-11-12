// 3GIM(V2) sample sketch for Mega/Leonardo.. -- httpGET

#include "a3gim2.h"

#define baudrate 	9600UL

const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)
const char *server = "www.arduino.cc";
const char *path = "";
const int port = a3gsDEFAULT_PORT;

char res[a3gsMAX_RESULT_LENGTH+1];
int len;

void setup()
{
  Serial.begin(baudrate);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
    Serial.print("httpGET() requesting.. ");
    len = sizeof(res);
    if (a3gs.httpGET(server, port, path, res, len, true) == 0) {
      Serial.println("OK!");
      Serial.print("[");
      Serial.print(res);
      Serial.println("]");
    }
    else {
      Serial.print("Can't get HTTP response from ");
      Serial.println(server);
    }
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
