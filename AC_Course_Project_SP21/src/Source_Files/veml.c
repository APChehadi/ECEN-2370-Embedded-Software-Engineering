/**
 * @file
 * 	veml.c
 * @author
 * 	Adam Chehadi
 * @date
 * 	04/28/2021
 * @brief
 *	Contains all of the veml driver functions
 */

#include "veml.h"

static uint32_t light_data;


/***************************************************************************//**
 * @brief
 *   VEML I2C Open Function
 *
 * @details
 * 	 Completes the configuration of a local struct of type I2C_OPEN_STRUCT. I2C_OPEN_STRUCT contains
 * 	 the information needed to complete the set up of the I2C external devices such as the I2C bus speed
 *
 * @note
 *   This function does not have any input or return values.
 *
 ******************************************************************************/
void veml_i2c_open() {
	I2C_OPEN_STRUCT i2c_init_values;

	i2c_init_values.enable = VEML_ENABLE;
	i2c_init_values.master = VEML_MASTER;
	i2c_init_values.refFreq = VEML_ref_freq;
	i2c_init_values.freq = VEML_FREQ;
	i2c_init_values.clhr = VEML_CLHR;
	i2c_init_values.out_pin_scl_route = VEML_SCL_ROUTE;
	i2c_init_values.out_pin_sda_route = VEML_SDA_ROUTE;
	i2c_init_values.out_pin_scl_en = true;
	i2c_init_values.out_pin_sda_en = true;

	i2c_open(VEML_I2C, &i2c_init_values);
}


/***************************************************************************//**
 * @brief
 *   VEML Read Function
 *
 * @details
 * 	 Calls i2c_start will proper initialization values to start the I2C peripheral
 *
 * @note
 *   This function does not have any return values. The input value is the external device
 *   callback in the event to be serviced.
 *
 ******************************************************************************/
void veml_read(uint32_t veml_read_cb) {
	i2c_start(VEML_I2C, VEML_ADDR, VEML_READ, VEML_RW_R, &light_data, VEML_CB, I2C_BYTES_2);
	timer_delay(15);
}


/***************************************************************************//**
 * @brief
 *   VEML Write Function
 *
 * @details
 * 	 Calls i2c_start will proper initialization values to start the I2C peripheral
 *
 * @note
 *   This function does not have any return values. The input value is the external device
 *   callback in the event to be serviced.
 *
 ******************************************************************************/
void veml_write() {
	i2c_start(VEML_I2C, VEML_ADDR, VEML_CONFIG, VEML_RW_W, &light_data, VEML_CB, I2C_BYTES_2);
	timer_delay(15);
}


/***************************************************************************//**
 * @brief
 *   VEML Lux Conversion Function
 *
 * @details
 * 	 This function takes the light sensor measurement from the I2C peripheral and converts that value
 * 	 into a float lux value.
 *
 * @note
 *   This function does not have any input values.
 *
 ******************************************************************************/
float compute_lux() {
	float result = light_data * 0.0576;
	return result;
}
