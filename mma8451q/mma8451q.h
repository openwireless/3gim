/*
 *	MMA8451Q.h
 *
 *	MMA8451Q control library for Arduino (Interface)
 *
 *	Version:
 *		R1.0  2017.01.09
 *		R1.1  2017.02.01  add getAccelerations()
 *		R1.2  2020.01.01  fix getFIFOEntries() and readFIFO() return wrong number
 *
 *	Note:
 *		MMA8451Q is a 14bit ADC 3-Axis MEMS Accelerometer.
 *
 *	Copyright(c) 2017 A.Daikoku
 */

#ifndef _MMA8451Q_
#define _MMA8451Q_

#include <Arduino.h>
#include <Wire.h>

/*
 *	Constants
 */
#define	m8MAX_FIFO_SAMPLES		32			// Maximum number of samples that can be held in the FIFO
		/* The "sample" is defined as one acceleration of one axis.
		   That is, the XYZ set of accelerations is assumed to be three samples.
		 */

/*
 *	Register Address
 */
	/**	Register_name				Register_No	   [mode] Default_value **/
#define	m8REG_STATUS				0x00		// [R] 0x00 if FMODE = 0 then real time status
#define	m8REG_F_STATUS				0x00		// [R] 0x00 if FMODE > 0 then FIFO status
#define	m8REG_OUT_X_MSB				0x01		// [R] -
#define	m8REG_OUT_X_LSB				0x02		// [R] -
#define	m8REG_OUT_Y_MSB				0x03		// [R] -
#define	m8REG_OUT_Y_LSB				0x04		// [R] -
#define	m8REG_OUT_Z_MSB				0x05		// [R] -
#define	m8REG_OUT_Z_LSB				0x06		// [R] -
#define	m8REG_F_SETUP				0x09		// [RW] 0x00
#define	m8REG_TRIG_CFG				0x0a		// [RW] 0x00
#define	m8REG_SYSMOD				0x0b		// [R] 0x00
#define	m8REG_INT_SOURCE			0x0c		// [R] 0x00
#define	m8REG_WHO_AM_I				0x0d		// [R] 0x1a(=device id)
#define	m8REG_XYZ_DATA_CFG			0x0e		// [RW] 0x00
#define	m8REG_HP_FILTER_CUTOFF		0x0f		// [RW] 0x00
#define	m8REG_PL_STATUS				0x10		// [R] 0x00
#define	m8REG_PL_CFG				0x11		// [RW] 0x80
#define	m8REG_PL_COUNT				0x12		// [RW] 0x00
#define	m8REG_PL_BF_ZCOMP			0x13		// [RW] 0x44
#define	m8REG_P_L_THS_REG			0x14		// [RW] 0x84
#define	m8REG_FF_MT_CFG				0x15		// [RW] 0x00
#define	m8REG_FF_MT_SRC				0x16		// [R] 0x00
#define	m8REG_FF_MT_THS				0x17		// [RW] 0x00
#define	m8REG_FF_MT_COUNT			0x18		// [RW] 0x00
#define	m8REG_TRANSIENT_CFG			0x1d		// [RW] 0x00
#define	m8REG_TRANSIENT_SRC			0x1e		// [R] 0x00
#define	m8REG_TRANSIENT_THS			0x1f		// [RW] 0x00
#define	m8REG_TRANSIENT_COUNT		0x20		// [RW] 0x00
#define	m8REG_PULSE_CFG				0x21		// [RW] 0x00
#define	m8REG_PULSE_SRC				0x22		// [R] 0x00
#define	m8REG_PULSE_THSX			0x23		// [RW] 0x00
#define	m8REG_PULSE_THSY			0x24		// [RW] 0x00
#define	m8REG_PULSE_THSZ			0x25		// [RW] 0x00
#define	m8REG_PULSE_TMLT			0x26		// [RW] 0x00
#define	m8REG_PULSE_LTCY			0x27		// [RW] 0x00
#define	m8REG_PULSE_WIND			0x28		// [RW] 0x00
#define	m8REG_ALSP_COUNT			0x29		// [RW] 0x00
#define	m8REG_CTRL_REG1				0x2a		// [RW] 0x00
#define	m8REG_CTRL_REG2				0x2b		// [RW] 0x00
#define	m8REG_CTRL_REG3				0x2c		// [RW] 0x00
#define	m8REG_CTRL_REG4				0x2d		// [RW] 0x00
#define	m8REG_CTRL_REG5				0x2e		// [RW] 0x00
#define	m8REG_OFF_X					0x2f		// [RW] 0x00
#define	m8REG_OFF_Y					0x30		// [RW] 0x00
#define	m8REG_OFF_Z					0x31		// [RW] 0x00

// REG_STATUS
#define	m8STATUS_ZYXOW				0x80		// [R] 0x00
#define	m8STATUS_ZYXD				0x08		// [R] 0x00

// REG_F_STATUS
#define	m8F_STATUS_F_OVF			0x80		// [R] 0x00
#define	m8F_STATUS_F_WMRK_FLAG		0x40		// [R] 0x00
#define	m8F_STATUS_F_CNT_MASK		0x3f		// [R] 0x00 (value 0-32)

// REG_F_SETUP
#define	m8F_SETUP_F_MODE_MASK		0xc0		// [RW] 0x00
#define	m8F_SETUP_F_WMRK_MASK		0x3f		// [RW] 0x00 (value 0-32)
	// m8F_SETUP_F_MODE_MASK's values
#define	m8F_SETUP_F_MODE_DISABLE	0x00
#define	m8F_SETUP_F_MODE_CIRC_BUF	0x40
#define	m8F_SETUP_F_MODE_FILL_BUF	0x80
#define	m8F_SETUP_F_MODE_TRIG_MODE	0xc0

// m8REG_TRIG_CFG
#define	m8TRIG_CFG_TRIG_TRANS		0x20		// [RW] 0x00
#define	m8TRIG_CFG_TRIG_LNDPRT		0x10		// [RW] 0x00
#define	m8TRIG_CFG_TRIG_PULSE		0x08		// [RW] 0x00
#define	m8TRIG_CFG_TRIG_FF_MT		0x04		// [RW] 0x00

// m8REG_SYSMODE
#define	m8SYSMOD_FGERR				0x80		// [R] 0x00
#define	m8SYSMOD_FGT_MASK			0x7c		// [R] 0x00 (value 0-31)
#define	m8SYSMOD_SYSMOD_MASK		0x03		// [R] 0x00
	// m8_SYSMOD_SYSMOD_MASK's values
#define	m8SYSMOD_SYSMOD_STANDBY_MODE 0x00
#define	m8SYSMOD_SYSMOD_WAKE_MODE	0x00
#define	m8SYSMOD_SYSMOD_SLEEP_MODE	0x00

// m8REG_INIT_SOURCE
#define m8INT_SOURCE_SRC_ASLP		0x80		// [R] 0x00
#define m8INT_SOURCE_SRC_FIFO		0x40		// [R] 0x00
#define m8INT_SOURCE_SRC_TRANS		0x20		// [R] 0x00
#define m8INT_SOURCE_SRC_LNDPRT		0x10		// [R] 0x00
#define m8INT_SOURCE_SRC_PULSE		0x08		// [R] 0x00
#define m8INT_SOURCE_SRC_FF_MT		0x04		// [R] 0x00
#define	m8INT_SOURCE_SRC_DRDY		0x01		// [R] 0x00

// m8REG_WHO_AM_I
#define	m8WHO_AM_I_MMA8451Q_ID		0x1a		// [R] 0x1a

// m8REG_XYZ_DATA_CFG
#define	m8XYZ_DATA_CFG_HPF_OUT		0x10		// [RW] 0x00
#define	m8XYZ_DATA_FS_MASK			0x03		// [RW] 0x00
	// m8XYZ_DATA_FS_MASK's values
#define	m8XYZ_DATA_FS_2G			0x00
#define	m8XYZ_DATA_FS_4G			0x01
#define	m8XYZ_DATA_FS_8G			0x02
#define	m8XYZ_DATA_FS_RESERVED		0x03

// m8REG_HP_FILTER_CUTOFF
#define	m8HP_FILTER_CUTOFF_HPF_BYP	0x20		// [RW] 0x00
#define	m8HP_FILTER_CUTOFF_LPF_EN	0x10		// [RW] 0x00
#define	m8HP_FILTER_CUTOFF_SEL_MASK	0x03		// [RW] 0x00
	// m8HP_FILTER_CUTOFF_SEL_MASK's values (when ODR=100Hz)
#define	m8HP_FILTER_CUTOFF_SEL_N4HZ		0X00		// @Normal mode
#define	m8HP_FILTER_CUTOFF_SEL_N2HZ		0X01
#define	m8HP_FILTER_CUTOFF_SEL_N1HZ		0X02
#define	m8HP_FILTER_CUTOFF_SEL_N05HZ	0X03
#define	m8HP_FILTER_CUTOFF_SEL_H16HZ	0X00		// @High resolution mode
#define	m8HP_FILTER_CUTOFF_SEL_H8HZ		0X01
#define	m8HP_FILTER_CUTOFF_SEL_H4HZ		0X02
#define	m8HP_FILTER_CUTOFF_SEL_H2HZ 	0X03

// m8REG_PL_STATUS
#define	m8PL_STATUS_NEWLP			0x80		// [R] 0x00
#define	m8PL_STATUS_LO				0x40		// [R] 0x00

// m8REG_PL_CFG
#define	m8PL_CFG_DBCNTM				0x80		// [RW] 0x80
#define	m8PL_CFG_PL_EN				0x40		// [RW] 0x00

// m8REG_CTRL_REG1
#define	m8CTRL_REG1_ASLP_RATE_MASK	0xc0		// [RW] 0x00 (value 0-3)
#define	m8CTRL_REG1_DR_MASK			0x38		// [RW] 0x00 (value 0-7)
#define	m8CTRL_REG1_LNOISE			0x04		// [RW] 0x00
#define	m8CTRL_REG1_F_READ			0x02		// [RW] 0x00
#define	m8CTRL_REG1_ACTIVE			0x01		// [RW] 0x00
	// m8CTRL_REG1_DR_MASK's values
#define	m8CTRL_REG1_DR_800HZ		0x00
#define	m8CTRL_REG1_DR_400HZ		0x08
#define	m8CTRL_REG1_DR_200HZ		0x10
#define	m8CTRL_REG1_DR_100HZ		0x18
#define	m8CTRL_REG1_DR_50HZ			0x20
#define	m8CTRL_REG1_DR_12_5HZ		0x28
#define	m8CTRL_REG1_DR_6_25HZ		0x30
#define	m8CTRL_REG1_DR_1_56HZ		0x38

// m8REG_CTRL_REG2
#define	m8CTRL_REG2_ST				0x80		// [RW] 0x00
#define	m8CTRL_REG2_RST				0x40		// [RW] 0x00
#define	m8CTRL_REG2_SMODE_MASK		0x18		// [RW] 0x00 (value 0-3)
#define	m8CTRL_REG2_SLPE			0x04		// [RW] 0x00
#define	m8CTRL_REG2_MODS_MASK		0x03		// [RW] 0x00 (value 0-3)
	// m8CTRL_REG2_SMODE_MASK's values
#define	m8CTRL_REG2_SMODE_NORMAL	0x00
#define	m8CTRL_REG2_SMODE_LOW_NOISE	0x08
#define	m8CTRL_REG2_SMODE_HIGH_RES	0x10
#define	m8CTRL_REG2_SMODE_LOW_POWER	0x18
	// m8CTRL_REG2_MODE_MASK's values
#define	m8CTRL_REG2_MODE_NORMAL		0x00
#define	m8CTRL_REG2_MODE_LOW_NOISE	0x01
#define	m8CTRL_REG2_MODE_HIGH_RES	0x02
#define	m8CTRL_REG2_MODE_LOW_POWER	0x03

// m8REG_CTRL_REG3
#define	m8CTRL_REG3_FIFO_GATE		0x80		// [RW] 0x00
#define	m8CTRL_REG3_WAKE_TRANS		0x40		// [RW] 0x00
#define	m8CTRL_REG3_WAKE_LNDRPT		0x20		// [RW] 0x00
#define	m8CTRL_REG3_WAKE_PULSE		0x10		// [RW] 0x00
#define	m8CTRL_REG3_WAKE_FF_MT		0x08		// [RW] 0x00
#define	m8CTRL_REG3_IPOL			0x02		// [RW] 0x00
#define	m8CTRL_REG3_PP_OD			0x01		// [RW] 0x00

// m8REG_CTRL_REG4
#define	m8CTRL_REG4_INT_EN_ASLP		0x80		// [RW] 0x00
#define	m8CTRL_REG4_INT_EN_FIFO		0x40		// [RW] 0x00
#define	m8CTRL_REG4_INT_EN_TRANS	0x20		// [RW] 0x00
#define	m8CTRL_REG4_INT_EN_LNDPRT	0x10		// [RW] 0x00
#define	m8CTRL_REG4_INT_EN_PULSE	0x08		// [RW] 0x00
#define	m8CTRL_REG4_INT_EN_FF_MT	0x04		// [RW] 0x00
#define	m8CTRL_REG4_INT_EN_DRY		0x01		// [RW] 0x00

// m8REG_CTRL_REG5
#define	m8CTRL_REG5_INT_CFG_ASLP	0x80		// [RW] 0x00
#define	m8CTRL_REG5_INT_CFG_FIFO	0x40		// [RW] 0x00
#define	m8CTRL_REG5_INT_CFG_TRANS	0x20		// [RW] 0x00
#define	m8CTRL_REG5_INT_CFG_LNDPRT	0x10		// [RW] 0x00
#define	m8CTRL_REG5_INT_CFG_PULSE	0x08		// [RW] 0x00
#define	m8CTRL_REG5_INT_CFG_FF_MT	0x04		// [RW] 0x00
#define	m8CTRL_REG5_INT_CFG_DRY		0x01		// [RW] 0x00

/*
 *	MMA8451Q class
 */
class MMA8451Q {
  public:
	MMA8451Q(int sa0 = HIGH);						// Constructor with SA0
	uint8_t readRegister(uint8_t reg);				// Read byte data from a register
	void readMultiRegisters(uint8_t reg, int bytes, uint8_t data[]);	// Read multi bytes data from registers
	void writeRegister(uint8_t reg, uint8_t data);	// Write byte data to a register
	int getAccelerations(int *ax, int *ay, int *az); // Get 3-axis accelerations
	int getFIFOEntries(void) __attribute__((always_inline)) {
		return ((readRegister(m8REG_F_STATUS) & m8F_STATUS_F_CNT_MASK));
	}                                               // Get number of Samples exist in FIFO
	int readFIFO(int samples[], int numSamples);	// Read data from FIFO
	int toRaw(uint8_t high, uint8_t low) __attribute__((always_inline)) {
		int8_t signedHigh = high;
		return ((((int)signedHigh << 8) + low) >> 2);
	}                                               // Convert to 14-bit accelaration
    int tomg(int raw, int range) __attribute__((always_inline)) {
		if (range == m8XYZ_DATA_FS_2G)
			return (raw / 4.096);	// 4096 LSB/g @+/-2g
		else if (range == m8XYZ_DATA_FS_4G)
			return (raw / 2.048);	// 2048 LSB/g @+/-4g
		else
			return (raw / 1.024);	// 1024 LSB/g @+/-8g
	}                                               // Convert raw to mg

  private:
	uint8_t	_i2cAddress;
};

#endif // _MMA8451Q_
