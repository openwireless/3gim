/*
 *  STTS751 sample sketch - Read temperature and print it to serial monitor
 *
 *	  MGIMボード上の温度センサSTTS751から、温度を読み取るサンプル
 */

#include <Wire.h>
#include <mgim.h>
#include <stts751.h>

STTS751 stts751;	// STTS751's Addr pin => GND

void setup() {
	mgim.begin();

	while (! mgSERIAL_MONITOR)
		;
	mgSERIAL_MONITOR.begin(9600);

    Wire.begin();
    stts751.begin();
}

void loop() {
	int tX10 = stts751.getTemperature();
	mgSERIAL_MONITOR.print(tX10 / 10);
	mgSERIAL_MONITOR.print(".");
	mgSERIAL_MONITOR.println(abs(tX10 % 10));

	delay(100);
}
