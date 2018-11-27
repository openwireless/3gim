// 4GIM(V1) sample sketch for Mega/Leonardo/Zero.. -- connectTCP/disconnectTCP/read/write
//   A title is extracted from a homepage. 

#include <a4gim2.h>

#define MAX_RETRY  3

const int powerPin = 7;     // 4gim power pin(If not using power control, 0 is set.)
const char *server = "tabrain.jp";  // URL to extract a title
const char *path = "/products/config/package_tabrain_index.json";
const int port = 80;
char res[200];

void setup()
{
  Serial.begin(9600);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

 _redo:
  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0) {
    Serial.println("Succeeded.");
    // Connect to server
    int nCount = 0;
    while (nCount++ < MAX_RETRY && a4gs.connectTCP(server, port) != 0) {
      Serial.println("connectTCP() can't connect, retry..");
      delay(100);
    }
    if (nCount > MAX_RETRY) {
      Serial.println("connectTCP() abort");
      goto _end;
    }
    // Send GET request
    a4gs.write("GET ");
    a4gs.write(path);
    a4gs.write(" HTTP/1.1$r$n");
    a4gs.write("HOST:");
    a4gs.write(server);
    a4gs.write("$r$n");
    a4gs.write("$r$n");
    // Receive response and print
    int nbytes;
    while ((nbytes = a4gs.read(res, sizeof(res)-1)) > 0) {
      for (int i = 0; i < nbytes; i++)
        Serial.print(res[i]);
    }
    Serial.println();
  }
  else {
    Serial.println("Failed.");
  }

  Serial.println("Shutdown..");
  a4gs.end();
  a4gs.shutdown();

  delay(15000);
  goto _redo;  // Repeat

 _end:
  while (1) ;
}

void loop()
{
}

// END