//   AT&T m2x cloud server upload example using a4gim
//   upload data are temperature data and illumination data on the IoTABshield V3.0 (produced by Tabrain Inc.)
//   設定  <DEVICE-ID＞ および ＜API-KEY＞　m2xで設定のこと
//   <STREAM ID> は、 温度センサ値「TEMP」および光センサ値「LIGHT｝の２つ m2xで設定のこと

#include "a4gim.h"

#define TEMPPRA    0.594   // IoTABシールド 5V系マイコンボードの場合
//#define TEMPPRA  0.392   //            3.3V系マイコンボードの場合

const char *server = "api-m2x.att.com";
const char *path = "v2/devices/<DEVICE-ID>/updates/";
const char *header="X-M2X-KEY:<API-KEY>$r$nContent-Type:application/json$r$n";
const int port = a4gsDEFAULT_PORT;
const char *bodyTemplate = "{$\"values$\" : {$\"TEMP$\" : [{$\"timestamp$\" : $\"%s$\", $\"value$\" : $\"%d.%d$\"}], $\"LIGHT$\" : [{$\"timestamp$\" : $\"%s$\", $\"value$\" : $\"%d$\"}] }}";
const int powerPin = 7;     // 4gim power pin
const int tempPin = 1, illPin = 0;  // Sensors pin number on IoTABshield V3.0 

void setup() {
 delay(100);  // Wait for Start Serial Monitor
 Serial.begin(9600);
 Serial.println(">Ready.");

  Serial.print("Initializing.. ");
  if (a4gs.start(powerPin) == 0 && a4gs.begin() == 0)
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

  char res[50], body[200]; 
  int stat, len;
  sprintf(body, bodyTemplate, datetime, (temp/10), (temp%10), datetime, ill);
  Serial.println(body);
  len = sizeof(res);
  if ((stat = a4gs.httpPOST(server, port, path, header, body, res, &len)) == 0) {
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
  char date[a4gsDATE_SIZE], time[a4gsTIME_SIZE];
  if (a4gs.getTime(date, time) != 0) {
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
