/*
 * SI7021.h
 *
 *  Created on: Mar 2, 2021
 *      Author: adamp
 */

#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_


/* Silicon Labs include statements */
#include "em_assert.h"
#include "em_int.h"
#include "em_i2c.h"


/* The developer's include statements */
#include "i2c.h"
#include "brd_config.h"
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define SI7021_FREQ				I2C_FREQ_FAST_MAX
#define SI7021_CLHR				i2cClockHLRAsymetric
#define SI7021_I2C				I2C1
#define SI7021_COMMAND			0xF5
#define SI7021_SLAVE_ADDRESS	0x40
#define SI7021_ENABLE			true
#define SI7021_MASTER			true
#define SI7021_REF_FREQ			0

#define SI7021_TEMP_NO_HOLD 	0xF3
#define SI7021_HUMI_NO_HOLD 	0xF5
#define SI7021_READ_USER_REG	0xE7
#define SI7021_WRITE_USER_REG	0xE6
#define SI7021_ADDR				0x40
#define SI7921_REG				0xE6
#define RESET_VAL				0b00111010

#define	TEMP_FROM_RH			0xE0

#define RES_CONFIG				0x01
#define RES_8_12_BIT			0x3B

//***********************************************************************************
// function prototypes
//***********************************************************************************
void si7021_i2c_open(void);
void si7021_read(uint32_t SI7021_read_cb);
void si7021_temp_read(uint32_t SI7021_read_cb);
float si7021_humidity_conversion();
float temperature_calculation();
bool i2c_test(uint32_t si7021_read_cb);

#endif /* SRC_HEADER_FILES_SI7021_H_ */
