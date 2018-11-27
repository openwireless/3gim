// 4GIM(V1) sample sketch for Mega/Leonardo.. -- use $ commands

#define  ioSerial     Serial
#define  gimSerial  Serial1

const unsigned long baudrate = 115200;

void setup() {
  ioSerial.begin(baudrate);
  gimSerial.begin(baudrate);

  ioSerial.println("Ready.");
}

void loop() {
  if (gimSerial.available() > 0) {
    char c = gimSerial.read();
    ioSerial.print(c);
  }

  if (ioSerial.available() > 0) {
    char c = ioSerial.read();
    ioSerial.print(c);    // Echo back
    gimSerial.print(c);
  }  
}
