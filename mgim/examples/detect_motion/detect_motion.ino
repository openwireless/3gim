/*
 * Detect motion sample sketch for MGIM
 *
 * [Description]
 *  This is a sample sketch that constantly detects acceleration of 64mg or more
 *  and turns on the LED for 2 seconds when detected.
 *  For continuous monitoring, the ODR is measured at 100 Hz, the measurement range is +/- 2g,
 *  low noise, and 14-bit.
 *  The MCU always sleeps using the sleep feature of the RTC and wakes up once every ALARM_INTERVAL minutes.
 *
 * [Note]
 *  The current consumption during sleep(MCU is standby mode and LIS2DW is active) is approximately 80-90uA.
 *  When writing this sketch for the second time or later, it is necessary to press
 *  the reset switch once immediately before writing to make the MCU recognize the MCU as a USB device.
 *  Do not use LTE-M communication function, so no SIM and antenna required.
 */

#include <Wire.h>
#include <RTCZero.h>
#include <mgim.h>
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
#define LED_ON_TIME       2000    // LED lighting time [mS]
#define ALARM_INTERVAL    3       // Alarm interval [min.]

// Initial settings
#define REG_CTRL1     (s2ODR_100 | s2MODE_LOW_POWER | s2LP_MODE_1)
#define REG_CTRL2     (s2CTRL2_BDU)
#define REG_CTRL3     (s2CTRL3_H_LACTIVE | s2CTRL3_LIR)
#define REG_CTRL6     (s2FS_2G | s2CTRL6_FDS | s2CTRL6_LOW_NOISE | s2BW_FILT_DIV2)
#define REG_CTRL7     (s2CTRL7_USR_OFF_ON_OUT)

// Define accelaration sensor and RTC instance
LIS2DW    acc(HIGH);   // Accelerometer
RTCZero   rtc;         // Real Time Clock in atsamd21

volatile int        ledOn = false;      // Current status of Led
volatile uint32_t   detectedTime = 0;   // last detected time

// setup() --
void setup(void) {
  mgim.begin();
  Wire.begin();

  // Check id
  delay(20);    // Wait for lis2dw to complete booting
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
  acc.writeRegister(s2REG_CTRL6, REG_CTRL6);
  delay(1);
  acc.writeRegister(s2REG_CTRL1, REG_CTRL1);
  delay(1);
  acc.writeRegister(s2REG_CTRL2, REG_CTRL2);
  delay(1);
  acc.writeRegister(s2REG_CTRL3, REG_CTRL3);
  delay(1);
  uint8_t reg = acc.readRegister(s2REG_CTRL7);
  acc.writeRegister(s2REG_CTRL7, reg | s2CTRL7_USR_OFF_ON_OUT);
  delay(1);
  reg = acc.readRegister(s2REG_WAKE_UP_DUR);
  acc.writeRegister(s2REG_WAKE_UP_DUR, reg | (s2WAKE_UP_DUR_WAKE_DUR_MASK & 0));
  // Set Wake-Up threshold: 1 LSb corresponds to FS_XL/2^6
  reg = acc.readRegister(s2REG_WAKE_UP_THS);
  acc.writeRegister(s2REG_WAKE_UP_THS, reg | (s2WAKE_UP_THS_WK_THS_MASK & 2));
  // Set route WU to INT1
  reg = acc.readRegister(s2REG_CTRL4_INT1_PAD_CTRL);
  acc.writeRegister(s2REG_CTRL4_INT1_PAD_CTRL, reg | s2CTRL4_INT1_WU);
  reg = acc.readRegister(s2REG_CTRL7);
  acc.writeRegister(s2REG_CTRL7, reg | s2CTRL7_INTERRUPTS_ENABLE);

  // Set up RTC (Set Mar/03/2020 00:00:00, for now)
  rtc.begin();
  rtc.setTime(0, 0, 0);
  rtc.setDate(3, 3, 2020);
  setNextAlarm();
  rtc.attachInterrupt(onAlarm);     // Enable alarm interrupt

  // Inform you that you are ready by Led
  blinkLed(3, 300);

  // Set up GPIO pins for sleep
  setupGPIOs();

  mgim.setAccelerationHandler(onMotion);
}

// loop() --
void loop(void) {
    if (ledOn && (millis() - detectedTime > LED_ON_TIME)) {
        ledOn = false;
        mgim.setLed(0);     // Turn off led
        acc.readRegister(s2REG_ALL_INT_SRC);  // clear INT1
    }

    if (! ledOn) {
        setupGPIOs();           // Set up GPIO pins for sleep
        rtc.standbyMode();      // Sleep(Stand by) now..
    }
}

void onMotion(void) {
    if (ledOn)
        return;

    uint8_t intSource = acc.readRegister(s2REG_ALL_INT_SRC);  // Read interrupt sources and cleat INT1
    if (intSource & s2INT_SRC_WU_IA) {
        uint8_t wuSource = acc.readRegister(s2REG_WAKE_UP_SRC);
        if (wuSource & (s2WAKE_UP_SRC_X_WU | s2WAKE_UP_SRC_Y_WU | s2WAKE_UP_SRC_Z_WU)) {
            detectedTime = millis();
            mgim.setLed(1);     // Turn on led
            ledOn = true;
        }
    }
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

// setupGPIOs() -- Set up GPIO pins for sleep(Minimize the outflow/inflow current from GPIO pins)
void setupGPIOs(void) {
    for (int i = 0; i <= 31; i++) {
      switch (i) {
          case _mgLedPin:
          case _mgINT1Pin:
          case _mgHL7800PowerPin:
          case _mgVinPin:
          case _mgHL7800WakeUpPin:
          case _mgHL7800ResetPin:
          case _mgHL7800PowerOnPin:
          case _mgHL7800VGPIOPin:
          case _mgHL7800ShutdownPin:
          case _mgHL7800CTS:
          case _mgHL7800RTS:
          case SDA:
          case SCL:
            break;
          default:
            pinMode(i, OUTPUT);
      }
    }
}
