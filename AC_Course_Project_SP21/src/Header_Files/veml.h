/*
 * veml.h
 *
 *  Created on: Apr 28, 2021
 *      Author: adamp
 */

#ifndef SRC_HEADER_FILES_VEML_H_
#define SRC_HEADER_FILES_VEML_H_

/* Silicon Labs include statements */
#include "em_i2c.h"


/* The developer's include statements */
#include "i2c.h"
#include "brd_config.h"
#include "HW_delay.h"
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define VEML_FREQ				I2C_FREQ_FAST_MAX;
#define VEML_CLHR				i2cClockHLRAsymetric
#define VEML_I2C				I2C0
#define VEML_MASTER				true
#define VEML_ENABLE				true
#define VEML_ref_freq			0

#define VEML_RW_R				true
#define VEML_RW_W				false

#define VEML_ADDR				0x48
#define VEML_READ				4
#define VEML_CONFIG				0x00

//***********************************************************************************
// function prototypes
//***********************************************************************************
void veml_i2c_open(void);
void veml_read(uint32_t veml_read_cb);
void veml_write(void);
float compute_lux(void);



#endif /* SRC_HEADER_FILES_VEML_H_ */
