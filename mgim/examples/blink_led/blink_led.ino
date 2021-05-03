/*
 *  mgim usage and blink led on board - sample sketch
 */

#include <mgim.h>

void setup() {
  mgim.begin();
}

void loop() {
  mgim.setLed(1);   // Turn on led
  delay(500);
  mgim.setLed(0);   // Turn off led
  delay(500);
}
