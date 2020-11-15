/*
 * Detect motion sample sketch for MGIM
 *
 * [Description]
 *  This is a sample sketch that constantly detects acceleration of 3g or more
 *  and turns on the LED for 2 seconds when detected.
 *  For continuous monitoring, the ODR is measured at 50 Hz, the measurement range is +/- 4g,
 *  low noise, and 14-bit.
 *  The MCU always sleeps using the sleep feature of the RTC and wakes up once every ALARM_INTERVAL minutes.
 *
 * [Note]
 *  When writing this sketch for the second time or later, it is necessary to press
 *  the reset switch once immediately before writing to make the MCU recognize the MCU as a USB device.
 *  Do not use LTE-M communication function, so no SIM and antenna required.
 */

#include <Wire.h>
#include <mgim.h>
#include <RTCZero.h>
#include <mma8451q.h>

#undef DEBUG_USB

#if DEBUG_USB
# define DBG(...)       { mgSERIAL_MONITOR.print(__VA_ARGS__); }
# define DBGLN(...)     { mgSERIAL_MONITOR.println(__VA_ARGS__); }
#else
# define DBG(...)
# define DBGLN(...)
#endif /* DEBUG_USB */

// Constants
#define CALIBRATE_COUNT   100
#define LED_ON_TIME       2000    // LED lighting time [mS]
#define ALARM_INTERVAL    3       // Alarm interval [min.]

// Initial settings
#define XYZ_DATA_CFG      (m8XYZ_DATA_FS_4G)
//#define CTRL_REG1         (m8CTRL_REG1_DR_50HZ)
#define CTRL_REG1         (m8CTRL_REG1_DR_12_5HZ)
//#define CTRL_REG2         (m8CTRL_REG2_MODE_HIGH_RES)
#define CTRL_REG2         (m8CTRL_REG2_MODE_NORMAL)
#define CTRL_REG3         (m8CTRL_REG3_PP_OD)
#define CTRL_REG4         (m8CTRL_REG4_INT_EN_FF_MT)
#define CTRL_REG5         (m8CTRL_REG5_INT_CFG_FF_MT)

const int range = m8XYZ_DATA_FS_4G;

// Define accelaration sensor and RTC instance
MMA8451Q  acc(LOW);    // Accelerometer
RTCZero   rtc;         // Real Time Clock in atsamd21

// For interrupts
volatile uint32_t  interruptedTime = 0;
volatile boolean   ledOn = false;

// setup() --
void setup(void) {
  if (mgim.begin() != mgSUCCESS) {
    DBGLN("mgim.begin(): error.");
    while (1) ;       // Stop here
  }

  Wire.begin();

  // Check id
  uint8_t id = acc.readRegister(m8REG_WHO_AM_I);
  if (id != m8WHO_AM_I_MMA8451Q_ID) {
    DBG("Bad id=0x");
    DBGLN(id, HEX);
    while (1) ;       // Stop here
  }

  // Reset MMA8451Q
  acc.writeRegister(m8REG_CTRL_REG2, m8CTRL_REG2_RST);
  while (acc.readRegister(m8REG_CTRL_REG2) & m8CTRL_REG2_RST)
    ;   // wait for ready

  // Set up MMA8451Q
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
  acc.writeRegister(m8REG_FF_MT_THS, 48);		  // Set motion detection of > 3g
  delay(10);
//  acc.writeRegister(m8REG_FF_MT_COUNT, 5);		  // Set debounce counter to eliminate false readings for 50 Hz sample rate with a requirement of 100 ms timer
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

  // Set up RTC (Set Mar/03/2020 00:00:00, for now)
  rtc.begin();
  rtc.setTime(0, 0, 0);
  rtc.setDate(3, 3, 2020);
  setNextAlarm();

  // Inform you that you are ready by Led
  blinkLed(3, 300);
  DBGLN("Ready.");

  // Turn off hl7800
  delay(10000);
  Serial.begin(115200);
  Serial.println("ATZ");
  delay(300);
  Serial.println("AT+CEDRXS=1");
  delay(300);
  Serial.println("AT+CPWROFF=1");
  Serial.flush();
  delay(300);
  Serial.end();
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  for (int i = 0; i < mgim.numberOfExternalPins; i++)
    pinMode(mgim.externalPins[i], OUTPUT);

  // Set interrupt handler
  mgim.setAccelarationHandler(onDetectMotion);
  rtc.attachInterrupt(onAlarm);     // Enable alarm interrupt
}

// loop() --
void loop(void) {
  if (ledOn && (millis() - interruptedTime > LED_ON_TIME)) {
    mgim.setLed(0);   // Turn off led
    ledOn = false;

    acc.readRegister(m8REG_FF_MT_SRC);  // Clear INT1
    delay(10);

    uint8_t reg1 = acc.readRegister(m8REG_CTRL_REG1);
    reg1 &= ~m8CTRL_REG1_ACTIVE;
    acc.writeRegister(m8REG_CTRL_REG1, reg1);  // Set standby mode
    delay(10);
    acc.writeRegister(m8REG_CTRL_REG4, CTRL_REG4);
    delay(10);
    reg1 |= m8CTRL_REG1_ACTIVE;
    acc.writeRegister(m8REG_CTRL_REG1, reg1);  // Set active mode

    mgim.setAccelarationHandler(onDetectMotion);  // Enable interrupt again
  }

  if (! ledOn)
    rtc.standbyMode();    // Sleep now..
}

// onDetectMotion() -- hendler on detect motion
void onDetectMotion(void) {
  mgim.setAccelarationHandler(0);        // Disable interrupt

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
    mgim.setLed(1);     // Turn on led
    ledOn = true;
  }
}

// setNextAlarm() -- set next alarm
void setNextAlarm(void) {
  int mm = rtc.getMinutes();
  mm += ALARM_INTERVAL;
  if (mm > 59)
    mm -= 60;
  rtc.setAlarmMinutes(mm);          // Set next wake up time
  rtc.setAlarmSeconds(0);
  rtc.enableAlarm(rtc.MATCH_MMSS);  // Enable periodic alarm
}

// onAlarm() -- handler on alaram
void onAlarm(void) {
  setNextAlarm();
}

// blinkLed() -- blink led on mgim
void blinkLed(int counts, int cycleTime) {
  while (counts-- > 0) {
    mgim.setLed(1);
    delay(cycleTime);
    mgim.setLed(0);
    delay(cycleTime);
  }
}
