/*
 *  STTS751 sample sketch for CAIM - Read temperature and print it to serial monitor
 */

#include <Wire.h>
#include <stts751.h>

#define SERIAL_MONITOR      SerialUSB

STTS751 stts751;

void setup() {
	while (! SERIAL_MONITOR)
		;
	SERIAL_MONITOR.begin(9600);

  Wire.begin();
  stts751.begin();
}

void loop() {
	int tX10 = stts751.getTemperature();
	SERIAL_MONITOR.print(tX10 / 10);
	SERIAL_MONITOR.print(".");
	SERIAL_MONITOR.println(abs(tX10 % 10));

	delay(100);
}
