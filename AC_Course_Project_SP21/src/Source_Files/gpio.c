/**
 * @file
 * 	gpio.c
 * @author
 * 	Adam Chehadi
 * @date
 * 	02/07/2021
 * @brief
 *	Contains gpio_open function
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Enable clock, LED configuration
 *
 * @details
 * 	 Set drive strength, GPIO pin mode set.
 *
 * @note
 *   Identify which GPIO port to access, pin number, and desired mode.
 *
 ******************************************************************************/
void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED0_PORT, LED0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, LED0_GPIOMODE, LED0_DEFAULT);

	GPIO_DriveStrengthSet(LED1_PORT, LED1_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, LED1_DEFAULT);


	// Configure I2C pins
	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, SI7021_DRIVE_STRENGTH);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, SI7021_SENSOR_GPIOMDOE, SI7021_SENSOR_DEFAULT);

	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, SI7021_I2C_GPIOMODE, SI7021_I2C_DEFAULT);

	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, SI7021_I2C_GPIOMODE, SI7021_I2C_DEFAULT);


	// Configure I2C pins
	GPIO_PinModeSet(VEML_SCL_PORT, VEML_SCL_PIN, VEML_SENSOR_GPIOMODE, VEML_I2C_DEFAULT);
	GPIO_PinModeSet(VEML_SDA_PORT, VEML_SDA_PIN, VEML_SENSOR_GPIOMODE, VEML_I2C_DEFAULT);


	// Configure UART pins
	GPIO_DriveStrengthSet(LEUART0_TX_PORT, LEUART0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LEUART0_TX_PORT, LEUART0_TX_PIN, LEUART0_TX_GPIOMODE, LEUART0_TX_DEFAULT);

	GPIO_PinModeSet(LEUART0_RX_PORT, LEUART0_RX_PIN, LEUART0_RX_GPIOMODE, LEUART0_RX_DEFAULT);
}
