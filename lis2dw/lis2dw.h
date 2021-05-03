/*
 *	LIS2DW.h
 *
 *	LIS2DW control library for Arduino (Interface)
 *
 *	Version:
 *		R1.0  2021.04.30
 *
 *	Note:
 *		LIS2DW is a 14bit ADC 3-Axis MEMS Accelerometer made by STmaicro.
 *
 *	Copyright(c) 2021 TABrain inc.
 */

#ifndef _LIS2DW_
#define _LIS2DW_

#include <Arduino.h>
#include <Wire.h>
/*
 *	Constants
 */
#define	s2MAX_FIFO_SAMPLES		32			// Maximum number of samples that can be held in the FIFO
		/* The "sample" is defined as one acceleration of three axis.
		   That is, the XYZ set of accelerations is assumed to be one samples.
		 */

/*
 *	Register Address
 */
	/**	Register_name				Register_No	   [mode] Default_value **/
#define s2REG_OUT_T_L               0x0d        // [R] Temperature sensor output(Low)
#define s2REG_OUT_T_H               0x0e        // [R] Temperature sensor output(High)
#define s2REG_WHO_AM_I              0x0f        // [R] Who am I ID - 0x44
#define s2REG_CTRL1                 0x20        // [R/W] Control register #1
#define s2REG_CTRL2                 0x21        // [R/W] Control register #2
#define s2REG_CTRL3                 0x22        // [R/W] Control register #3
#define s2REG_CTRL4_INT1_PAD_CTRL   0x23        // [R/W] Control register #4
#define s2REG_CTRL5_INT2_PAD_CTRL   0x24        // [R/W] Control register #5
#define s2REG_CTRL6                 0x25        // [R/W] Control register #6
#define s2REG_OUT_T                 0x26        // [R] Temperature sensor output(8-bit)
#define s2REG_STATUS                0x27        // [R] Status data register
#define s2REG_OUT_X_L               0x28        // [R] X-axis Output register(Low)
#define s2REG_OUT_X_H               0x29        // [R] X-axis Output register(High)
#define s2REG_OUT_Y_L               0x2a        // [R] Y-axis Output register(Low)
#define s2REG_OUT_Y_H               0x2b        // [R] Y-axis Output register(High)
#define s2REG_OUT_Z_L               0x2c        // [R] Z-axis Output register(Low)
#define s2REG_OUT_Z_H               0x2d        // [R] Z-axis Output register(High)
#define s2REG_FIFO_CTRL             0x2e        // [R/W] FIFO control register
#define s2REG_FIFO_SMPLES           0x2f        // [R] Unread samples stored in FIFO
#define s2REG_TAP_THS_X             0x30        // [R/W] Tap threshold X-axis
#define s2REG_TAP_THS_Y             0x31        // [R/W] Tap threshold Y-axis
#define s2REG_TAP_THS_Z             0x32        // [R/W] Tap threshold Z-axis
#define s2REG_INT_DUR               0x33        // [R/W] Interrupt duration
#define s2REG_WAKE_UP_THS           0x34        // [R/W] Wake up threshold..
#define s2REG_WAKE_UP_DUR           0x35        // [R/W] Wake up duration
#define s2REG_FREE_ALL              0x36        // [R/W] Free-fall configuration
#define s2REG_STATUS_DUP            0x37        // [R] Status register
#define s2REG_WAKE_UP_SRC           0x38        // [R] Wake up source
#define s2REG_TAP_SRC               0x39        // [R] Tap source
#define s2REG_SIXD_SRC              0x3a        // [R] 6D source
#define s2REG_ALL_INT_SRC           0x3b        // [R] All interrupts source
#define s2REG_X_OFS_USR             0x3c        // [R] User offset X-axis
#define s2REG_Y_OFS_USR             0x3d        // [R] User offset Y-axis
#define s2REG_Z_OFS_USR             0x3e        // [R] User offset Z-axis
#define s2REG_CTRL7                 0x3f        // [R/W] Control register #7

// REG_OUT_T_L
#define s2OUT_T_L_MASK              0xf0        // Tmperature low byte mask

// REG_WHO_AM_I
#define s2WHO_AM_I_ID               0x44        // Who am I ID

// REG_CTRL1
#define s2CTRL1_ODR_MASK            0xf0        // Output date rate configuration(s2CTRL1_ODR_*) - 0000
#define s2CTRL1_MODE_MASK           0x0c        // Mode selection(s2MODE_*) - 00
#define s2CTRL1_LP_MODE_MASK        0x03        // Low power mode selection(s2LP_MODE_*) - 00
  // Output data rate configuration
#define s2ODR_POWER_DOWN            0x00        // ODR None
#define s2ODR_1_6                   0x01        // ODR 12.5Hz(HP)/1.6Hz(LP)
#define s2ODR_12_5                  0x20        // ODR 12.5Hz(HP/LP)
#define s2ODR_25                    0x30        // ODR 25Hz(HP/LP)
#define s2ODR_50                    0x40        // ODR 50Hz(HP/LP)
#define s2ODR_100                   0x50        // ODR 100Hz(HP/LP)
#define s2ODR_200                   0x60        // ODR 200Hz(HP/LP)
#define s2ODR_400                   0x70        // ODR 400Hz(HP)/200Hz(LP)
#define s2ODR_800                   0x80        // ODR 800Hz(HP)/200Hz(LP)
#define s2ODR_1600                  0x90        // ODR 1600Hz(HP)/200Hz(LP)
  // Mode selection
#define s2MODE_LOW_POWER            0x00        // Mode: Low power mode(12/14-bit)
#define s2MODE_HIGH_PERFORMANCE     0x04        // Mode: High performance mode(14-bit)
#define s2MODE_SINGLE_ON_DEMAND     0x08        // Mode: Simgle data conversion on deman mode(12/14-bit)
#define s2MODE_LOW_RESERVED         0x0C        // Mode: Reserved
  // Low-power mode selection
#define s2LP_MODE_1                 0x00        // Low-power mode: #1 (12-bit)
#define s2LP_MODE_2                 0x01        // Low-power mode: #2 (14-bit)
#define s2LP_MODE_3                 0x02        // Low-power mode: #3 (14-bit)
#define s2LP_MODE_4                 0x03        // Low-power mode: #4 (14-bit)

// REG_CTRL2
#define s2CTRL2_BOOT                0x80        // Boot enable(0:disable/1:enable) - 0
#define s2CTRL2_SOFT_RESET          0x40        // Soft reset(0:disable/1:enable) - 0
#define s2CTRL2_CS_PU_DISC          0x10        // Disconnect CS pull-up(0:pull-up/1:disconnect pull-up) - 0
#define s2CTRL2_BDU                 0x08        // Block data update(0:continuous update/1: output registers not updated until MSB and LSB read) - 0
#define s2CTRL2_IF_ADD_INC          0x04        // Register address automatically incremented during multiple byte access(0:disable/1:enable) - 0
#define s2CTRL2_I2C_DISABLE         0x02        // Disable i2c communication(0:SPI and i2c enable/1:i2c disable)
#define s2CTRL2_SIM                 0x01        // SPI serial interface mode selection(0:4-wire interface/1:3-wire interface)

// REG_CTRL3
#define s2CTRL3_ST_MASKS            0xc0        // Selt-Test enable and mode(s2CTRL3_ST_*) - 00
#define s2CTRL3_PP_OD               0x02        // Push-pull/open-drain selection on interrupt pad(0:push-pull/1:open-drain) - 0
#define s2CTRL3_LIR                 0x01        // Latched Interrupt(0:interrupt request not latched/1:interrupt request latched - 0
#define s2CTRL3_H_LACTIVE           0x08        // Interrupt active high, low(0:active high/1:active low)
#define s2CTRL3_SLP_MODE_SEL        0x02        // Single data conversion on demand mode selection(0:enabled with INT2/1:enabled by I²C/SPI writing SLP_MODE_1 to 1)
#define s2CTRL3_SLIP_MODE_1         0x01        // Single data conversion on demand mode(0: automaticaly/1:When SLP_MODE_SEL = '1' and this bit is set to '1' logic, single data conversion on demand mode starts)
  // ST(Self-Test) mode selection
#define s2CTRL3_ST_NORMAL           0x00        // Self-test: Normal mode
#define s2CTRL3_ST_POSITIVE_SIGN    0x40        // Self-test: Positive sign self-test
#define s2CTRL3_ST_NEGATIVE_SIGN    0x80        // Self-test: Negative sign self-test
#define s2CTRL3_ST_RESERVED         0xc0        // Self-test: Reserved

// REG_CTRL4_IN1_PAD_CTRL
#define s2CTRL4_INT1_6D             0x80        // 6D recognition is routed to INT1 pad(0:disable/1:enable) - 0
#define s2CTRL4_INT1_SINGLE_TAP     0x40        // Single-tap recognition is routed to INT1 pad(0:disable/1:enable) - 0
#define s2CTRL4_INT1_WU             0x20        // Wakeup recognition is routed to INT1 pad(0:disable/1:enable) - 0
#define s2CTRL4_INT1_FF             0x10        // Free-fall recognition is routed to INT1 pad(0:disable/1:enable) - 0
#define s2CTRL4_INT1_TAP            0x08        // Double-tap recognition is routed to INT1(0:disable/1:enable) - 0
#define s2CTRL4_INT1_DIFF5          0x04        // FIFO full recognition is routed to INT1 pad(0:disable/1:enable) - 0
#define s2CTRL4_INT1_FTH            0x02        // FIFO threshold interrupt is routed to INT1 pad(0:disable/1:enable) - 0
#define s2CTRL4_INT1_DRDY           0x01        // Data-Ready is routed to INT1 pad(0:disable/1:enable) - 0

// REG_CTRL5_INT2_PAD_CTRL
#define s2CTRL5_INT2_SLEEP_STATE    0x08        // Enable routing of SLEEP_STATE on INT2 pad(0:disable/1:enable) - 0
#define s2CTRL5_INT2_SLEEP_CHG      0x04        // Sleep change status routed to INT2 pad(0:disable/1:enable) - 0
#define s2CTRL5_INT2_BOOT           0x02        // Boot state routed to INT2 pad(0:disable/1:enable) - 0
#define s2CTRL5_INT2_DRDY_T         0x10        // Temperature data-ready is routed to INT2(0:disable/1:enable) - 0
#define s2CTRL5_INT2_OVR            0x80        // FIFO overrun interrupt is routed to INT2 pad(0:disable/1:enable) - 0
#define s2CTRL5_INT2_DIFF5          0x40        // FIFO full recognition is routed to INT2 pad(0:disable/1:enable) - 0
#define s2CTRL5_INT2_FTH            0x02        // FIFO threshold interrupt is routed to INT2 pad(0:disable/1:enable) - 0
#define s2CTRL5_INT2_DRDY           0x00        // Data-ready is routed to INT2 pad(0:disable/1:enable) - 0

// REG_CTRL6(Table 42)
#define s2CTRL6_BW_FILT_MASK        0xc0        // Bandwidth selection(see s2BW_FILT_*) - 00
#define s2CTRL6_FS_MASK             0x30        // Full-scale selection(see s2FS_*) - 00
#define s2CTRL6_FDS                 0x08        // Filtered data type selection(0:low-pass filter/1:high-pass filter) - 0
#define s2CTRL6_LOW_NOISE           0x04        // Low-noise configuration(0:disable/1:enable) - 0
  // Bandwidth selection
#define s2BW_FILT_DIV2              0x00        // ODR/2 (@)
#define s2BW_FILT_DIV4              0x40        // ODR/4 (HP/LP)
#define s2BW_FILT_DIV10             0x80        // ODR/10 (HP/LP)
#define s2BW_FILT_DIV20             0xc0        // ODR/16 (HP/LP)
  // Full-scale selection
#define s2FS_2G                     0x00        // +/- 2g
#define s2FS_4G                     0x10        // +/- 4g
#define s2FS_8G                     0x20        // +/- 8g
#define s2FS_16G                    0x30        // +/- 16g

// REG_STATUS
#define s2STATUS_FIFO_THS           0x80        // FIFO threshold status flag(0:FIFO filling is lower than threshold/1:FIFO filling is equal to or higher than the threshold)
#define s2STATUS_WU_IA              0x40        // Wakeup event detection status(0:Wakeup event not detected/1:Wakeup event detected)
#define s2STATUS_SLEEP_STATE        0x20        // Sleep event status(0:Sleep event not detected/1:Sleep event detected)
#define s2STATUS_DOUBLE_TAP         0x10        // Double-tap event status(0:Double-tap event not detected/1:Double-tap event detected)
#define s2STATUS_SINGLE_TAP         0x08        // Single-tap event status(0:Single-tap event not detected/1:Single-tap event detected)
#define s2STATUS_6D_IA              0x04        // Source of change in position portrait/landscape/face-up/face-down(0:no event detected/1:a change in position detected)
#define s2STATUS_FF_IA              0x02        // Free-fall event detection status(0:free-fall event not detected/1:free-fall event detected)
#define s2STATUS_DRDY               0x01        // Data-ready status(0:not ready/1:X-,Y- and Z-axis new data available)

// REG_FIFO_CTRL(Table 56)
#define s2FIFO_CTRL_FMODE_MASK      0xe0        // FIFO mode selection(s2FMODE_*) - 000
#define s2FIFO_CTRL_FTH_MASK        0x1f        // FIFO threshold level setting(max 31)
  // FIFO mode selection(Table 58)
#define s2FMODE_BYPASS              0x00        // Bypass mode: FIFO turned off
#define s2FMODE_FIFO                0x20        // FIFO mode: Stops collecting data when FIFO is full
#define s2FMODE_CONTINUOUS_FIFO     0x50        // Continuous-to-FIFO: Stream mode until trigger is deasserted, then FIFO mode
#define s2FMODE_BYPASS2CONTINUOUS   0x80        // Bypass-to-Continuous: Bypass mode until trigger is deasserted, then FIFO mode
#define s2FMODE_CONTINUOUS          0xc0        // Continuous mode: If the FIFO is full, the new sample overwrites the older sample

// REG_FIFO_SAMPLES(table 59)
#define s2FIFO_SAMPLES_FIFO_FTH     0x80        // FIFO threshold status flag(0:FIFO filling is lower than threshold/1:FIFO filling is equal to or higher than the threshold)
#define s2FIFO_SAMPLES_FIFO_OVR     0x40        // FIFO overrun status(0:FIFO is not completely filled/1:FIFO is completely filled and at least one sample has been overwritten)
#define s2FIFO_SAMPLES_DIFF_MASK    0x3f        // Represents the number of unread samples stored in FIFO(000000:FIFO empty/100000:FIFO full, 32 unread samples)

// REG_TAP_THS_X
// REG_TAP_THS_Y
// REG_TAP_THS_Z

// REG_INT_DUR(Table 69)
#define s2INT_DUR_LATENCY_MASK      0xf0        // Duration of maximum time gap for double-tap recognition(1 LSB = 32 * 1/ODR) - 0000(which is 16 * 1/ODR)
#define s2INT_DUR_QUIET_MASK        0x0c        // Expected quiet time after a tap detection(1 LSB = 4 * 1/ODR) - 00(which is 2 * 1/ODR)
#define s2INT_DUR_SHOCK_MASK        0x03        // Maximum duration of over-threshold event(1 LSB = 8 * 1/ODR) - 00(which is 4 * 1/ODR)

// REG_WAKE_UP_THS(Table 71)
#define s2WAKE_UP_THS_SINGLE_DOUBLE_TAP 0x00    // Enable single/double-tap event(0:only single-tap event is enabled/1:single and double-tap events are enabled) - 0
#define s2WAKE_UP_THS_SLEEP_ON      0x00        // Sleep (inactivity) enable(0:sleep disabled/1:sleep enabled) - 0
#define s2WAKE_UP_THS_WK_THS_MASK   0x3f        // Wakeup threshold(6-bit unsigned, 1 LSB = 1/64 of FS) - 000000

// REG_WAKE_UP_DUR(Table #define s2WAKE_UP_DUR_
#define s2WAKE_UP_DUR_FF_DUR5       0x80        // Free-fall duration. In conjunction with FF_DUR [4:0] bit(1 LSB = 1 * 1/ODR) - 0
#define s2WAKE_UP_DUR_WAKE_DUR_MASK 0x60        // Wakeup duration.(1 LSB = 1 * 1/ODR) - 00
#define s2WAKE_UP_DUR_STATIONARY    0x10        // Enable stationary detection / motion detection with no automatic ODR change when detecting stationary state(0:disable/1:enable) - 0
#define s2WAKE_UP_DUR_SLEEP_DUR_MASK 0x0f       // Duration to go in sleep mode(1 LSB = 512 * 1/ODR) - 0000(which is 16 * 1/ODR)

// REG_FREE_FALL(Table 75)
#define s2FREE_FALL_FF_DUR_MASK     0xf0        // Free-fall duration. In conjunction with FF_DUR5 bit(1 LSB = 1 * 1/ODR)
#define s2FREE_FALL_FF_THS_MASK     0x0f        // Free-fall threshold @ FS = ±2 g (Table 77)

// REG_STATUS_DUP(Table 78)
#define s2STATUS_DUP_OVR            0x80        // FIFO overrun status flag(0:FIFO is not completely filled/1:FIFO is completely filled and at least one sample has been overwritten)
#define s2STATUS_DUP_DRDY_T         0x40        // Temperature status(0:data not available/1:a new set of data is available)
#define s2STATUS_DUP_SLEEP_STATE_IA 0x20        // Sleep event status(0:Sleep event not detected/1:Sleep event detected)
#define s2STATUS_DUP_DOUBLE_TAP     0x10        // Double-tap event status(0:Double-tap event not detected/1:Double-tap event detected)
#define s2STATUS_DUP_SINGLE_TAP     0x08        // Single-tap event status(0: ingle-tap event not detected/1:Single-tap event detected)
#define s2STATUS_DUP_6D_IA          0x04        // Source of change in position portrait/landscape/face-up/face-down(0:no event detected/1:a change in position is detected)
#define s2STATUS_DUP_FF_IA          0x02        // Free-fall event detection status(0:free-fall event not detected/1:free-fall event detected)
#define s2STATUS_DUP_DRDY           0x01        // Data-ready status(0:not ready/1:X-, Y- and Z-axis new data available)

// REG_WAKE_UP_SRC(Table 80)
#define s2WAKE_UP_SRC_FF_IA         0x20        // Free-fall event detection status(0:FF event not detected/1:FF event detected)
#define s2WAKE_UP_SRC_SLEEP_STATE_IA 0x10       // Sleep event status(0:Sleep event not detected/1:Sleep event detected)
#define s2WAKE_UP_SRC_WU_IA         0x80        // Wakeup event detection status(0:Wakeup event not detected/1:Wakeup event is detected)
#define s2WAKE_UP_SRC_X_WU          0x40        // Wakeup event detection status on X-axis(0:Wakeup event on X not detected/1:Wakeup event on X-axis is detected)
#define s2WAKE_UP_SRC_Y_WU          0x20        // Wakeup event detection status on Y-axis(0:Wakeup event on Y not detected/1:Wakeup event on Y-axis is detected)
#define s2WAKE_UP_SRC_Z_WU          0x10        // Wakeup event detection status on Z-axis(0:Wakeup event on Z not detected/1:Wakeup event on Z-axis is detected)

// REG_TAP_SRC(Table 82)
#define s2TAP_SRC_TAP_IA            0x40        // Tap event status(0:tap event not detected/1:tap event detected)
#define s2TAP_SRC_SINGLE_TAP        0x20        // Single-tap event status(0:single-tap event not detected/1:single-tap event detected)
#define s2TAP_SRC_DOUBLE_TAP        0x10        // Double-tap event status(0:double-tap event not detected/1:double-tap event detected)
#define s2TAP_SRC_TAP_SIGN          0x08        // Sign of acceleration detected by tap event(0:positive sign of acceleration detected/1:negative sign of acceleration detected)
#define s2TAP_SRC_X_TAP             0x04        // Tap event detection status on X-axis(0:Tap event on X not detected/1:Tap event on X-axis is detected)
#define s2TAP_SRC_Y_TAP             0x02        // Tap event detection status on Y-axis(0:Tap event on Y not detected/1:Tap event on Y-axis is detected)
#define s2TAP_SRC_Z_TAP             0x01        // Tap event detection status on Z-axis(0:Tap event on Z not detected/1:Tap event on Z-axis is detected)

// REG_SIXD_SRC(Table 84)
#define s2SIXD_SRC_6D_IA            0x40        // Source of change in position portrait/landscape/face-up/face-down(0:no event detected/1:a change in position is detected)
#define s2SIXD_SRC_ZH               0x20        // ZH over threshold(0:ZH does not exceed the threshold/1:ZH is over the threshold)
#define s2SIXD_SRC_ZL               0x10        // ZL over threshold(0:ZL does not exceed the threshold/1:ZL is over the threshold)
#define s2SIXD_SRC_YH               0x08        // YH over threshold(0:YH does not exceed the threshold/1:YH is over the threshold)
#define s2SIXD_SRC_YL               0x04        // YL over threshold(0:YL does not exceed the threshold/1:YL is over the threshold)
#define s2SIXD_SRC_XH               0x02        // XH over threshold(0:XH does not exceed the threshold/1:XH is over the threshold)
#define s2SIXD_SRC_XL               0x01        // XL over threshold(0:XL does not exceed the threshold/1:XL is over the threshold)

// ALL_INT_SRC(Table 86)
#define s2INT_SRC_SLEEP_CHANGE_IA   0x20        // Sleep change status(0:Sleep change not detected/1:Sleep change detected) - 0
#define s2INT_SRC_6D_IA             0x10        // Source of change in position portrait/landscape/face-up/face-down(0:no event detected/1:a change in position detected) - 0
#define s2INT_SRC_DOIUBLE_TAP       0x08        // Double-tap event status(0:double-tap event not detected/1:double-tap event detected) - 0
#define s2INT_SRC_SIMGLE_TAP        0x04        // Single-tap event status(0:single-tap event not detected/1:single-tap event detected) - 0
#define s2INT_SRC_WU_IA             0x02        // Wakeup event detection status(0:wakeup event not detected/1:wakeup event detected) - 0
#define s2INT_SRC_FF_IA             0x01        // Free-fall event detection status(0:free-fall event not detected/1:free-fall event detected) - 0

// REG_CTRL7(Table 94)
#define s2CTRL7_DRDY_PULSED         0x80        // Switches between latched and pulsed mode for data ready interrupt(0:latched mode is used/1:pulsed mode enabled for data-ready) - 0
#define s2CTRL7_INT2_ON_INT1        0x40        // Signal routing(0: no routing/1:all signals available only on INT2 are routed on INT1)
#define s2CTRL7_INTERRUPTS_ENABLE   0x20        // Enable interrupts(0:disable/1:enable) - 0
#define s2CTRL7_USR_OFF_ON_OUT      0x10        // Enable application of user offset value on XL output data registers. FDS bit in REG_CTRL6 must be set to '0'-logic (low-pass path selected)
#define s2CTRL7_USR_OFF_ON_WU       0x08        // Enable application of user offset value on XL data for wakeup function only(0:disable/1:enable) - 0
#define s2CTRL7_USR_OFF_W           0x04        // Selects the weight of the user offset words specified by X_OFS_USR,Y_OFS_USR and Z_OFS_USR(0:977µg/LSB; 1:15.6mg/LSB) - 0
#define s2CTRL7_HP_REF_MODE         0x02        // High-pass filter reference mode enable(0:high-pass filter reference mode disabled/1:high-pass filter reference mode enabled) - 0
#define s2CTRL7_LPASS_ON6D          0x01        // (0:ODR/2 low pass filtered data sent to 6D interrupt function/1:LPF2 output data sent to 6D interrupt function) - 0


/*
 *	LIS2DW class
 */
class LIS2DW {
  public:	LIS2DW(int sa0 = HIGH);						// Constructor with SA0
	uint8_t readRegister(uint8_t reg);				// Read byte data from a register
	void readMultiRegisters(uint8_t reg, int bytes, uint8_t data[]);	// Read multi bytes data from registers
	void writeRegister(uint8_t reg, uint8_t data);	// Write byte data to a register
	int getAccelerations(int *ax, int *ay, int *az); // Get 3-axis accelerations(12/14-bit)
	int getFIFOEntries(void) __attribute__((always_inline)) {
		return ((readRegister(s2REG_FIFO_SMPLES) & s2FIFO_SAMPLES_DIFF_MASK));
	}                                               // Get number of Samples exist in FIFO
	int readFIFO(int samples[], int numSamples);	// Read data from FIFO
	int toRaw(uint8_t high, uint8_t low) __attribute__((always_inline)) {
		int8_t signedHigh = high;
		return ((((int)signedHigh << 8) + low) >> 2);
	}                                               // Convert to 14-bit accelaration
    int tomg(int raw, int range) __attribute__((always_inline)) {
		switch (range) {
          case s2FS_2G:
			return (raw / 4.096);	// 4096 LSB/g @+/-2g
		  case s2FS_4G:
			return (raw / 2.048);	// 2048 LSB/g @+/-4g
		  case s2FS_8G:
			return (raw / 1.024);	// 1024 LSB/g @+/-8g
          default: // s2FS_16G
            return (raw / 0.512);	// 512 LSB/g @+/-8g
        }
	}                                               // Convert raw(14-bit) to mg(~gal)

  private:
	uint8_t	_i2cAddress;
};

#endif // _LIS2DW_
