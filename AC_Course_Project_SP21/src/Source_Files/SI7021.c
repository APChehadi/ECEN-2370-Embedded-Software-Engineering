/**
 * @file
 * 	app.c
 * @author
 * 	Adam Chehadi
 * @date
 * 	02/02/2021
 * @brief
 *	Contains si7021_i2c_open, si7021_read, si7021_humidity_conversion
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "SI7021.h"


//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t humidity_data;

//***********************************************************************************
// Private function prototypes
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   SI7021 I2C Open Function
 *
 * @details
 * 	 Completes the configuration of a local struct of type I2C_OPEN_STRUCT. I2C_OPEN_STRUCT contains
 * 	 the information needed to complete the set up of the I2C external devices such as the I2C bus speed
 *
 * @note
 *   This function does not have any input or return values.
 *
 ******************************************************************************/
void si7021_i2c_open() {
	I2C_OPEN_STRUCT i2c_init_values;

	i2c_init_values.enable = SI7021_ENABLE;
	i2c_init_values.master = SI7021_MASTER;
	i2c_init_values.refFreq = SI7021_REF_FREQ;
	i2c_init_values.freq = SI7021_FREQ;
	i2c_init_values.clhr = SI7021_CLHR;

	i2c_init_values.out_pin_scl_route = SI7021_SCL_ROUTE;
	i2c_init_values.out_pin_sda_route = SI7021_SDA_ROUTE;
	i2c_init_values.out_pin_scl_en = true;
	i2c_init_values.out_pin_sda_en = true;

	i2c_open(SI7021_I2C, &i2c_init_values);
}


/***************************************************************************//**
 * @brief
 *   SI7021 Read Function
 *
 * @details
 * 	 Calls i2c_start will proper initialization values to start the I2C peripheral
 *
 * @note
 *   This function does not have any return values. The input value is the external device
 *   callback in the event to be serviced.
 *
 ******************************************************************************/
void si7021_read(uint32_t SI7021_read_cb) {
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_COMMAND, true, &humidity_data, SI7021_READ_CB, I2C_BYTES_2);
	timer_delay(15);
}


/***************************************************************************//**
 * @brief
 *   SI7021 Temperature Read Function
 *
 * @details
 * 	 Calls i2c_start will proper initialization values to start the I2C peripheral
 *
 * @note
 *   This function does not have any return values. The input value is the external device
 *   callback in the event to be serviced.
 *
 ******************************************************************************/
void si7021_temp_read(uint32_t SI7021_read_cb) {
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, TEMP_FROM_RH, true, &humidity_data, SI7021_TEMP_READ_CB, I2C_BYTES_2);
	timer_delay(15);
}

/***************************************************************************//**
 * @brief
 *   SI7021 Humidity Conversion Function
 *
 * @details
 * 	 This function takes the humidity measurement from the I2C peripheral and converts that value
 * 	 into a float humidity percentage.
 *
 * @note
 *   This function does not have any input values.
 *
 ******************************************************************************/
float si7021_humidity_conversion() {
	float result = humidity_data;
	result = (125.0 * result) / 65536.0 - 6.0;
	return result;
}


/***************************************************************************//**
 * @brief
 *   SI7021 Temperature Conversion Function
 *
 * @details
 *   This function takes the humidity measurement from the I2C peripheral and converts that value
 * 	 into a float temperature value in farheneit.
 *
 * @note
 *   This function does not have any input values.
 *
 ******************************************************************************/
float temperature_calculation() {
	float result = humidity_data;
	result = ((175.72 * result) / 65536) - 46.85; // Celsius
	return (result * 1.8 + 32); // Fahrenheit
}


/***************************************************************************//**
 * @brief
 *   I2C Test Function
 *
 * @details
 *   This function is used to test read and write abilities with the SI7021 and I2C. The tests
 *   involve reading the default value of the user register, writing to user register then reading
 *   the value to make sure to write worked (1-byte op), and reading the humidity and temperature
 *   using the SI7021 (2-byte op). For each test, the i2c_start() is called with the proper parameters,
 *   then an EFM_ASSERT() is placed to verify that the operation completed.
 *
 * @note
 *   There are 15ms delays after each i2c_start() function is finished, so that the board has time to reset
 *   before the next operation is called.
 *
 * @param[in] si7021_read_cb
 *   call back when operation is done
 *
 * @return
 *   returns true if successfully passed all tests
 *
 ******************************************************************************/
bool i2c_test(uint32_t si7021_read_cb) {
	bool success = false;

	// Test Read Of User Register 1
	bool read_write = true; // read
	uint32_t previous_value = humidity_data;
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_READ_USER_REG, read_write, &humidity_data, SI7021_READ_CB, I2C_BYTES_1);
	while(check_busy(SI7021_I2C));
	timer_delay(15);
	EFM_ASSERT(humidity_data == RESET_VAL || humidity_data == previous_value);

	// Test Write To User Register 1
	humidity_data = RES_CONFIG;
	read_write = false; //write
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_WRITE_USER_REG, read_write, &humidity_data, SI7021_READ_CB, I2C_BYTES_1);
	while(check_busy(SI7021_I2C));
	timer_delay(15);
	EFM_ASSERT(humidity_data == RES_CONFIG);

	// Read Register Back To Make Sure Write Occurred
	read_write = true; //read
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_READ_USER_REG, read_write, &humidity_data, SI7021_READ_CB, I2C_BYTES_1);
	while(check_busy(SI7021_I2C));
	timer_delay(15);
	EFM_ASSERT(humidity_data == RES_8_12_BIT);

	// Test A 2-Byte Access Of The Humidity Reading
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_HUMI_NO_HOLD, read_write, &humidity_data, SI7021_READ_CB, I2C_BYTES_2);
	while(check_busy(SI7021_I2C));
	timer_delay(15);
	int humidity = si7021_humidity_conversion();
	EFM_ASSERT((humidity >= 20) && (humidity <= 60));

	// Test A 2-Byte Access Of The Temperature Reading
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_TEMP_NO_HOLD, read_write, &humidity_data, SI7021_READ_CB, I2C_BYTES_2);
	while(check_busy(SI7021_I2C));
	timer_delay(15);
	int temperature = temperature_calculation();
	EFM_ASSERT((temperature >= 30) && (temperature <= 100));

	success = true;

	return success;
}
