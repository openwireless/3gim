// 4GIM(V1) sample sketch for Mega/Leonardo.. -- httpGET

#include "a4gim2.h"

const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)
const char *server = "www.arduino.cc";
const char *path = "";
const int port = a4gsDEFAULT_PORT;

char res[a4gsMAX_RESULT_LENGTH+1];
int len;

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
    Serial.println("Succeeded.");
    Serial.print("httpGET() requesting.. ");
    len = sizeof(res);
    if (a4gs.httpGET(server, port, path, res, len, true) == 0) {
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
  a4gs.end();
  a4gs.shutdown();
}

void loop()
{
}

// END
