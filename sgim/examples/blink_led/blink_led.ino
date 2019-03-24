/*
 * Sample sketch for SGIM -- Blink Led on SGIM
 * 
 * [説明] 
 *   SGIM上のLEDを点滅させる
 *   
 */
#include <sgim.h>

void setup() {
  sgim.begin();
}

void loop() {
  sgim.setLed(1);
  delay(200);
  sgim.setLed(0);
  delay(200);
}