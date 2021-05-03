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
#include <lis2dw.h>

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
// Initial settings
#define REG_CTRL1     (s2ODR_200 | s2MODE_HIGH_PERFORMANCE)
#define REG_CTRL2     (s2CTRL2_BDU)
#define REG_CTRL6     (s2FS_2G | s2CTRL6_LOW_NOISE | s2BW_FILT_DIV2)

const int range = s2FS_2G;

// Define accelaration sensor and RTC instance
LIS2DW    acc(HIGH);   // Accelerometer
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
  delay(10);    // Wait for lis2dw to complete booting
  uint8_t id = acc.readRegister(s2REG_WHO_AM_I);
  if (id != s2WHO_AM_I_ID) {
    DBG("Bad id=0x");
    DBGLN(id, HEX);
    while (1) ;       // Stop here
  }

  // Reset LIS2DW
  acc.writeRegister(s2REG_CTRL2, s2CTRL2_SOFT_RESET);
  while (acc.readRegister(s2REG_CTRL2) & s2CTRL2_SOFT_RESET)
    ;   // wait for ready

  // Set up LIS2DW
  acc.writeRegister(s2REG_CTRL1, REG_CTRL1);
  delay(1);
  acc.writeRegister(s2REG_CTRL2, REG_CTRL2);
  delay(1);
  acc.writeRegister(s2REG_CTRL6, REG_CTRL6);
  delay(1);
  //
  uint8_t reg = acc.readRegister(s2REG_CTRL7);
  acc.writeRegister(s2REG_CTRL7, reg | s2CTRL7_USR_OFF_ON_OUT);
  //
  reg = acc.readRegister(s2REG_WAKE_UP_DUR);
  acc.writeRegister(s2REG_WAKE_UP_DUR, reg | (s2WAKE_UP_DUR_WAKE_DUR_MASK & 0));
  // Set Wake-Up threshold: 1 LSb corresponds to FS_XL/2^6
  reg = acc.readRegister(s2REG_WAKE_UP_THS);
  acc.writeRegister(s2REG_WAKE_UP_THS, reg | (s2WAKE_UP_THS_WK_THS_MASK & 2));
  // Set route WU to INT1
  reg = acc.readRegister(s2REG_CTRL4_INT1_PAD_CTRL);
  acc.writeRegister(s2REG_CTRL4_INT1_PAD_CTRL, reg | s2CTRL4_INT1_WU);

  // Set up RTC (Set Mar/03/2020 00:00:00, for now)
  rtc.begin();
  rtc.setTime(0, 0, 0);
  rtc.setDate(3, 3, 2020);
  setNextAlarm();

  // Inform you that you are ready by Led
  blinkLed(3, 300);
  DBGLN("Ready.");

  for (int i = 0; i < mgim.numberOfExternalPins; i++)
    pinMode(mgim.externalPins[i], OUTPUT);

  // Set interrupt handler
  mgim.setAccelerationHandler(onDetectMotion);
  rtc.attachInterrupt(onAlarm);     // Enable alarm interrupt
}

// loop() --
void loop(void) {
  if (ledOn && (millis() - interruptedTime > LED_ON_TIME)) {
    mgim.setLed(0);   // Turn off led
    ledOn = false;

    // Clear INT1
    acc.readRegister(s2REG_ALL_INT_SRC);

    // Set route WU to INT1
    uint8_t reg = acc.readRegister(s2REG_CTRL4_INT1_PAD_CTRL);
    acc.writeRegister(s2REG_CTRL4_INT1_PAD_CTRL, reg | s2CTRL4_INT1_WU);

    mgim.setAccelerationHandler(onDetectMotion);  // Enable interrupt again
  }

  if (! ledOn)
    rtc.standbyMode();    // Sleep now..
}

// onDetectMotion() -- hendler on detect motion
void onDetectMotion(void) {
  mgim.setAccelerationHandler(0);        // Disable interrupt

  uint8_t intSource = acc.readRegister(s2REG_ALL_INT_SRC);  // Read interrupt sources and cleat INT1/2
  if (intSource & s2INT_SRC_WU_IA) {
      uint8_t wuSource = acc.readRegister(s2REG_WAKE_UP_SRC);
      if (wuSource & (s2WAKE_UP_SRC_X_WU | s2WAKE_UP_SRC_Y_WU | s2WAKE_UP_SRC_Z_WU)) {

          // Set route WU to INT1
          uint8_t reg = acc.readRegister(s2REG_CTRL4_INT1_PAD_CTRL);
          acc.writeRegister(s2REG_CTRL4_INT1_PAD_CTRL, reg & ~s2CTRL4_INT1_WU);

          interruptedTime = millis();
          mgim.setLed(1);     // Turn on led
          ledOn = true;
      }
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
