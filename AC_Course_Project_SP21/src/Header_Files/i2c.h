/*
 * i2c.h
 *
 *  Created on: Feb 25, 2021
 *      Author: adamp
 */

#ifndef SRC_HEADER_FILES_I2C_H_
#define SRC_HEADER_FILES_I2C_H_

#include <stdint.h>
#include <stdbool.h>

#include "em_assert.h"
#include "em_int.h"
#include "em_i2c.h"
#include "em_cmu.h"
#include "app.h"

#include "sleep_routines.h"
#include "scheduler.h"

#define I2C_EM_BLOCK		EM2
#define I2C_READ			true
#define I2C_WRITE			false

#define I2C_BYTES_1			1
#define I2C_BYTES_2			2

typedef struct {
	bool					enable;
	bool					master;
	uint32_t				refFreq;
	uint32_t				freq;
	I2C_ClockHLR_TypeDef	clhr;

	uint32_t				out_pin_scl_route;
	uint32_t				out_pin_sda_route;
	bool					out_pin_scl_en;
	bool					out_pin_sda_en;
} I2C_OPEN_STRUCT;


typedef struct {
	uint32_t				current_state;
	I2C_TypeDef			    *I2Cx;
	uint32_t				slave_address;
	uint32_t				slave_register;
	bool					read_write;
	uint32_t				num_transfer_bytes;
	uint32_t				bytes_transfered;
	uint32_t				*data;
	uint32_t				si_cb;
	volatile bool			i2c_busy;
} I2C_STATE_MACHINE;

void i2c_open(I2C_TypeDef * i2c, I2C_OPEN_STRUCT * i2c_setup);

void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);

void i2c_start(I2C_TypeDef *i2cx, uint32_t slave_address, uint32_t slave_register, bool read_write, uint32_t *data, uint32_t si_read_cb, uint32_t num_bytes);
bool check_busy(I2C_TypeDef * i2c);

#endif /* SRC_HEADER_FILES_I2C_H_ */
