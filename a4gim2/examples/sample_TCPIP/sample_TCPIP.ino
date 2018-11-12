// 3GIM(V2) sample sketch for Mega/Leonardo/Zero.. -- connectTCP/disconnectTCP/read/write
//   A title is extracted from a homepage. 

#include <a3gim2.h>

#define baudrate 	        9600UL
#define MAX_RETRY  3

const int powerPin = 7;     // 3gim power pin(If not using power control, 0 is set.)
const char *server = "tabrain.jp";  // URL to extract a title
const char *path = "/new/";
const int port = 80;
char res[200];

boolean getTitle(char *res);

void setup()
{
  Serial.begin(baudrate);
  delay(3000);  // Wait for Start Serial Monitor
  Serial.println("Ready.");

 _redo:
  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0, baudrate) == 0) {
    Serial.println("Succeeded.");
    // Connect to server
    int nCount = 0;
    while (nCount++ < MAX_RETRY && a3gs.connectTCP(server, port) != 0) {
      Serial.println("connectTCP() can't connect, retry..");
      delay(100);
    }
    if (nCount > MAX_RETRY) {
      Serial.println("connectTCP() abort");
      goto _end;
    }
    // Send GET request
    a3gs.write("GET ");
    a3gs.write(path);
    a3gs.write(" HTTP/1.0$r$n");
    a3gs.write("HOST:");
    a3gs.write(server);
    a3gs.write("$r$n$r$n");
    // Receive response
    do {
      int nbytes;
      if ((nbytes = a3gs.read(res, sizeof(res)-1)) < 0) {
        Serial.println("read() failed");
        break;
      }
    } while (! getTitle(res));
  }
  else
    Serial.println("Failed.");

  Serial.println("Shutdown..");
  a3gs.end();
  a3gs.shutdown();

  delay(15000);
  goto _redo;  // Repeat

 _end:
  while (1) ;
}

void loop()
{
}

boolean getTitle(char *p)
{
  char  *title;

  while (*p != '\0') {
    if (*p++ != '<')
      continue;    // skip not tag
    if (strncmp((const char *)p, "title>", 6))
      continue;    // skip not title tag
    // title tag found
    p += 6;
    title = p;
    while (*p != '\0' && *p != '<')
      p++;
    *p = '\0';
    Serial.print(server);
    Serial.print(" : Page title is \"");
    Serial.print(title);
    Serial.println("\"");
    return true;
  }
  return false;
}

// END
