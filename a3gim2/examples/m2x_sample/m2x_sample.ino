//   AT&T m2x cloud server upload example using a3gim.h
//   upload data are temperature data and illumination data on the IoTABshield V3.0 (produced by Tabrain Inc.)
//   設定  <DEVICE-ID＞ および ＜API-KEY＞　m2xで設定のこと
//   <STREAM ID> は、 温度センサ値「TEMP」および光センサ値「LIGHT｝の２つ m2xで設定のこと

#include "a3gim2.h"
#define BAUDRATE 9600UL 
#define TEMPPRA 0.594   // IoTABシールド ５V系マイコンボードの場合
//#define TEMPPRA 0.392 //           ３．３V系マイコンボードの場合

const char *server = "api-m2x.att.com";
const char *path = "v2/devices/<DEVICE-ID>/updates/";
const char *header="X-M2X-KEY:<API-KEY>$r$nContent-Type:application/json$r$n";
const int port = a3gsDEFAULT_PORT;
const char *bodyTemplate = "{$\"values$\" : {$\"TEMP$\" : [{$\"timestamp$\" : $\"%s$\", $\"value$\" : $\"%d.%d$\"}], $\"LIGHT$\" : [{$\"timestamp$\" : $\"%s$\", $\"value$\" : $\"%d$\"}] }}";
const int powerPin = 7;     // 3gim power pin
const int tempPin = 1, illPin = 0;  // Sensors pin number on IoTABshield V3.0 
char res[100], body[256]; 

int len;

void setup() {
 delay(100);  // Wait for Start Serial Monitor
 Serial.begin(9600);
 Serial.println(">Ready.");

  Serial.print("Initializing.. ");
  if (a3gs.start(powerPin) == 0 && a3gs.begin(0,BAUDRATE) == 0)
    Serial.println("Succeeded.");
  else {
    Serial.println("Failed.");
    while (1) ;  // STOP
  }
}
void loop() {
  char datetime[30];
  Serial.print(">httpPOST requesting: ");
  if (getDateAndTime(datetime) != 0) {
    Serial.println("getDateAndTime() Failed.");
    return;
  }
  int temp = getTemperature();
  int ill = getIlluminance();

  sprintf(body, bodyTemplate, datetime, (temp/10), (temp%10), datetime, ill);
  Serial.println(body);
  len = sizeof(res);

  int stat;
  if ((stat = a3gs.httpPOST(server, port, path, header, body, res, &len)) == 0) {
    Serial.println("Succeeded.");
    Serial.print(">Response=[");
    Serial.print(res);
    Serial.println("]");
  }
  else {
    Serial.print("Failed: ");
    Serial.println(stat);
  }
  
  delay(60000);  // take an interval
}
int getDateAndTime(char *datetime) {
  char date[a3gsDATE_SIZE], time[a3gsTIME_SIZE];
  if (a3gs.getTime(date, time) != 0) {
    Serial.println("getTime() Failed.");
    return 1;    // error
  }
  date[4] = '-';
  date[7] = '-';
  sprintf(datetime, "%sT%s$+09:00", date, time);
  return 0;    // ok  
}

int getTemperature(void) {   // get temperature(x10 C)
  for(int i=0; i<10; i++) analogRead(A1); // 空読み
  return (207.26 - TEMPPRA*analogRead(A1))*10.0; // IoTABシールド V3.0利用の場合 
}

int getIlluminance(void) {  // get illuminance(x1 lux)
  float mV = analogRead(illPin) * 3.23;        // IoTABシールド V3.0利用の場合
  return (mV / 2.9);    
}

