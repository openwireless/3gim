/*
 * detect_motion.ino -- Detect motion sample with RTC
 * 
 * [説明]
 *  常時、3g以上の加速度を検知して、検知したら2秒間だけLEDを点灯する2ンプルスケッチ。
 *  常時の監視では、ODRが50Hz、計測レンジは+/-4g、ローノイズ、14-bitで計測を行う
 *  MCUはRTCのスリープ機能を使って常時スリープし、ALARM_INTERVAL分ごとに一度起きる。
 *
 * [注意点]
 *  常時スリープするため、SerialUSBへのプリント出力はできない。
 *  2回目以降、このスケッチを書き込む際は、書込み直前に一度リセットスイッチを押して、
 *  MCUをUSBデバイスとしてPCに認識させる必要がある。
 *
 */

#include <Wire.h>
#include <RTCZero.h>
#include <sgim.h>
#include <mma8452q.h>

// Constants
#define CALIBRATE_COUNT   100
#define LED_ON_TIME       2000    // LED lighting time [mS]
#define ALARM_INTERVAL    3       // Alarm interval [min.]

// Initial settings
#define XYZ_DATA_CFG      (m8XYZ_DATA_FS_4G)
#define CTRL_REG1         (m8CTRL_REG1_DR_12_5HZ)
#define CTRL_REG2         (m8CTRL_REG2_MODE_NORMAL)
#define CTRL_REG3         (m8CTRL_REG3_PP_OD)
#define CTRL_REG4         (m8CTRL_REG4_INT_EN_FF_MT)
#define CTRL_REG5         (m8CTRL_REG5_INT_CFG_FF_MT)

const int range = m8XYZ_DATA_FS_4G;

// Pin difinitions
const int pushSwitchPin = 12;
const int slideSwitchPin = 11;

// Define accelaration sensor and RTC instance
MMA8452Q  acc(LOW);    // Accelerometer
RTCZero   rtc;         // Real Time Clock in atsamd21
uint32_t  lastNotified = 0;

// For interrupts
volatile uint32_t  interruptedTime = 0;
volatile boolean   ledOn = false;

void setup() {
  if (! sgim.begin()) {
    blinkLed(5, 300);
    while (1)
      ;       // Stop here
  }

  pinMode(pushSwitchPin, INPUT_PULLUP);
  pinMode(slideSwitchPin, INPUT_PULLUP);

  // Sleep SFM10R3
  delay(500);
  if (! sgim.sleep()) {
    blinkLed(7, 300);
    while (1)
      ;       // Stop here
  }

  Wire.begin();

  // Check sensor id
  uint8_t id = acc.readRegister(m8REG_WHO_AM_I);
  if (id != m8WHO_AM_I_MMA8452Q_ID) {
    blinkLed(6, 300);
    while (1) 
      ;       // Stop here
  }

  // Reset MMA8452Q
  acc.writeRegister(m8REG_CTRL_REG2, m8CTRL_REG2_RST);
  while (acc.readRegister(m8REG_CTRL_REG2) & m8CTRL_REG2_RST)
    ;   // wait for ready

  // Set up MMA8452Q
  acc.writeRegister(m8REG_XYZ_DATA_CFG, XYZ_DATA_CFG);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG1, CTRL_REG1);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG2, CTRL_REG2);
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG3, CTRL_REG3);
  delay(10);
    // Set motion detect parameters
  acc.writeRegister(m8REG_FF_MT_CFG, 0xD8);		  // Set OAE=1, enabling X, Y, and the latch
  delay(10);
  acc.writeRegister(m8REG_FF_MT_THS, 24);		  // Set motion detection of > 2g
  delay(10);
  acc.writeRegister(m8REG_FF_MT_COUNT, 2);      // Set debounce counter to eliminate false readings for 50 Hz sample rate with a requirement of 100 ms timer
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG4, CTRL_REG4);  // Enable Motion/Freefall Interrupt Function
  delay(10);
  acc.writeRegister(m8REG_CTRL_REG5, CTRL_REG5);  // Route the Motion/Freefall Interrupt Function to INT1
  delay(10);

  // Start measurement!
  uint8_t reg1 = acc.readRegister(m8REG_CTRL_REG1);
  reg1 |= m8CTRL_REG1_ACTIVE;
  acc.writeRegister(m8REG_CTRL_REG1, reg1);    // and Go!  

  // Calibrate accelarations
  int tx = 0, ty = 0, tz = 0;
  for (int n = 0; n < CALIBRATE_COUNT; n++) {
    int ax, ay, az;
    acc.getAccelerations(&ax, &ay, &az);
    tx += ax;
    ty += ay;
    tz += az;    
  }
  tx /= CALIBRATE_COUNT;
  tx = tx >> 3;
  acc.writeRegister(m8REG_OFF_X, (- tx & 0xff));
  ty /= CALIBRATE_COUNT;
  ty = ty >> 3;
  acc.writeRegister(m8REG_OFF_Y, (- ty & 0xff));
  tz /= CALIBRATE_COUNT;
  tz = tz >> 3;
  tz -= 4095;
  acc.writeRegister(m8REG_OFF_Z, (- tz & 0xff));

  // Set up RTC
  rtc.begin();
  rtc.setTime(12, 00, 00);
  rtc.setDate(6, 17, 2019);
  setNextAlarm();

  // Inform you that you are ready by Led
  blinkLed(3, 500);

  // Set unused pins as output mode to save current consumption
  for (int i = 0; i < 10; i++) {
    switch (i) {
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        break;
      default:
        pinMode(i, OUTPUT);
        break;
    }
  }
  
  // Set interrupt handler
  sgim.onDetectMotion(onInterrupt);
  rtc.attachInterrupt(onAlarm);     // Enable alarm interrupt
}

void loop() {
  if (ledOn && (millis() - interruptedTime > LED_ON_TIME)) {
    notifyCloud(100, 200, 300);  // Notify the cloud service using Sigfox

    resetMe();
  }

  if (! ledOn) {
    rtc.standbyMode();    // Sleep now..
  }
}

void onInterrupt() {
  // Disable interrupt
  sgim.onDetectMotion(NULL);

  uint8_t intSource = acc.readRegister(m8REG_INT_SOURCE);
  if (intSource & m8INT_SOURCE_SRC_FF_MT) {
    acc.readRegister(m8REG_FF_MT_SRC);         // Clear INT1

    uint8_t reg1 = acc.readRegister(m8REG_CTRL_REG1);
    reg1 &= ~m8CTRL_REG1_ACTIVE;
    acc.writeRegister(m8REG_CTRL_REG1, reg1);  // Set standby mode  
    acc.writeRegister(m8REG_CTRL_REG4, 0);     // Disable FF/MT INT
    reg1 |= m8CTRL_REG1_ACTIVE;
    acc.writeRegister(m8REG_CTRL_REG1, reg1);  // Sert active mode  
    
    interruptedTime = millis();
    sgim.setLed(1);     // Turn on  led
    ledOn = true;
  }
}

void setNextAlarm() {
  int mm = rtc.getMinutes();
  mm += ALARM_INTERVAL;
  if (mm > 59)
    mm -= 60;
  rtc.setAlarmMinutes(mm);          // Set next wake up time
  rtc.setAlarmSeconds(0);
  rtc.enableAlarm(rtc.MATCH_MMSS);  // Enable periodic alarm
}

void onAlarm(void) {
  setNextAlarm();
}

void blinkLed(int counts, int cycleTime) {
  while (counts-- > 0) {
    sgim.setLed(1);
    delay(cycleTime);
    sgim.setLed(0);
    delay(cycleTime);
  }
}

// resetMe() - Soft reset, reboot me
void resetMe(void) {
    NVIC_SystemReset();      // processor software reset  
}
